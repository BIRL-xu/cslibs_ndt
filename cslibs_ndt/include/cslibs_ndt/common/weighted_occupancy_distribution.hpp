#ifndef CSLIBS_NDT_COMMON_WEIGHTED_OCCUPANCY_DISTRIBUTION_HPP
#define CSLIBS_NDT_COMMON_WEIGHTED_OCCUPANCY_DISTRIBUTION_HPP

#include <mutex>

#include <cslibs_math/statistics/weighted_distribution.hpp>
#include <cslibs_gridmaps/utility/inverse_model.hpp>

#include <cslibs_indexed_storage/storage.hpp>

namespace cslibs_ndt {
template<std::size_t Dim>
class EIGEN_ALIGN16 WeightedOccupancyDistribution
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    using allocator_t               = Eigen::aligned_allocator<WeightedOccupancyDistribution>;

    using Ptr                       = std::shared_ptr<WeightedOccupancyDistribution<Dim>>;
    using distribution_container_t  = WeightedOccupancyDistribution<Dim>;
    using distribution_t            = cslibs_math::statistics::WeightedDistribution<Dim, 3>;
    using distribution_ptr_t        = typename distribution_t::Ptr;
    using point_t                   = typename distribution_t::sample_t;

    inline WeightedOccupancyDistribution() :
        num_free_(0),
        weight_free_(0)
    {
    }

    inline WeightedOccupancyDistribution(const std::size_t num_free,
                                         const double      weight_free) :
        num_free_(num_free),
        weight_free_(weight_free)
    {
    }

    inline WeightedOccupancyDistribution(const std::size_t    num_free,
                                         const double         weight_free,
                                         const distribution_t data) :
        num_free_(num_free),
        weight_free_(weight_free),
        distribution_(new distribution_t(data))
    {
    }

    inline WeightedOccupancyDistribution(const WeightedOccupancyDistribution &other) :
        num_free_(other.num_free_),
        weight_free_(other.weight_free_),
        distribution_(other.distribution_),
        occupancy_(other.occupancy_),
        inverse_model_(other.inverse_model_)
    {
    }

    inline WeightedOccupancyDistribution& operator = (const WeightedOccupancyDistribution &other)
    {
        num_free_      = other.num_free_;
        weight_free_   = other.weight_free_;
        distribution_  = other.distribution_;
        occupancy_     = other.occupancy_;
        inverse_model_ = other.inverse_model_;
        return *this;
    }

    inline void updateFree(const std::size_t& num_free = 1.0, const double &weight_free = 1.0)
    {
        num_free_     += num_free;
        weight_free_  += weight_free;
        inverse_model_ = nullptr;
    }

    inline void updateOccupied(const point_t & p, const double& w = 1.0)
    {
        if (!distribution_)
            distribution_.reset(new distribution_t());

        distribution_->add(p, w);
        inverse_model_ = nullptr;
    }

    inline void updateOccupied(const distribution_ptr_t &d)
    {
        if (!d)
            return;

        if (!distribution_)
            distribution_.reset(new distribution_t());

        *distribution_ += *d;
        inverse_model_ = nullptr;
    }

    inline double weightFree() const
    {
        return weight_free_;
    }

    inline std::size_t numFree() const
    {
        return num_free_;
    }

    inline double weightOccupied() const
    {
        return distribution_ ? distribution_->getWeight() : 0.0;
    }

    inline double getOccupancy(const cslibs_gridmaps::utility::InverseModel::Ptr &inverse_model) const
    {
        if (!inverse_model)
            throw std::runtime_error("inverse model not set!");

        return getOccupancy(*inverse_model);
    }

    inline double getOccupancy(const cslibs_gridmaps::utility::InverseModel &inverse_model) const
    {
        if (&inverse_model == inverse_model_)
            return occupancy_;

        inverse_model_ = &inverse_model;
        occupancy_ = distribution_ ?
                    cslibs_math::common::LogOdds::from(
                        weight_free_ * inverse_model_->getLogOddsFree() +
                        distribution_->getWeight() * inverse_model_->getLogOddsOccupied() -
                        static_cast<double>(num_free_ + distribution_->getSampleCount()) * inverse_model_->getLogOddsPrior()) :
                    cslibs_math::common::LogOdds::from(
                        weight_free_ * inverse_model_->getLogOddsFree() -
                        static_cast<double>(num_free_) * inverse_model_->getLogOddsPrior());
        return occupancy_;
    }

    inline const distribution_ptr_t &getDistribution() const
    {
        return distribution_;
    }

    inline distribution_ptr_t &getDistribution()
    {
        return distribution_;
    }

    inline void merge(const WeightedOccupancyDistribution&)
    {
    }

    inline std::size_t byte_size() const
    {
        return distribution_ ? (sizeof(*this) + sizeof(distribution_t)) : sizeof(*this);
    }

private:
    std::size_t        num_free_;
    double             weight_free_;
    distribution_ptr_t distribution_;

    mutable double                                        occupancy_ = 0;
    mutable const cslibs_gridmaps::utility::InverseModel* inverse_model_ = nullptr; // may point to invalid memory!
};
}

#endif // CSLIBS_NDT_COMMON_WEIGHTED_OCCUPANCY_DISTRIBUTION_HPP
