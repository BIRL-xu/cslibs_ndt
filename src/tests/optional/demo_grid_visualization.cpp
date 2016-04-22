/// PROJECT
#include "visualize.hpp"
#include "../../data/pointcloud.hpp"

void linspace(const double min,
              const double max,
              const double res,
              std::vector<double> &values)
{
    const double        range = max - min;
    const std::size_t   intervals = range / res;
    for(std::size_t i = 0 ; i < intervals ; ++i) {
        values.push_back(min + res * i);
    }
}

typedef ndt::NDTMultiGrid2D::Point Point;

int main(int argc, char *argv[])
{
    std::vector<Point> points;
    /// generate horizontal lines
    std::vector<double> xs;
    linspace(-10.0, -1.0, 0.1, xs);
    for(double &e : xs) {
        points.push_back(Point(e, 0.0));
        points.push_back(Point(e, -2.0));
    }
    /// generate vertial lines
    std::vector<double> ys;
    linspace(-10.0, 10.0, 0.1, ys);
    for(double &e : ys) {
        points.push_back(Point(1.5, e));
        if(e < -2.0 || e > 0.0)
            points.push_back(Point(-1.0, e));
    }

    ndt::data::Pointcloud<2> pointcloud(points);
    std::cout << "min " << pointcloud.min << std::endl;
    std::cout << "max " << pointcloud.max << std::endl;

    ndt::NDTMultiGrid2D::Size   size = {20, 20};
    ndt::NDTMultiGrid2D::Resolution resolution = {1.0, 1.0};

    cv::Mat display = cv::Mat(800, 800, CV_8UC3, cv::Scalar());
    ndt::renderPoints(points, size, resolution, display);
    cv::imshow("display", display);
    cv::waitKey(0);

    /// no we put the multi grid into play
    ndt::NDTMultiGrid2D multigrid(size, resolution, Point(-10, -10));
    for(Point &p : points) {
        if(!multigrid.add(p))
            std::cerr << "could not add point" << std::endl;
    }
    if(!multigrid.add(pointcloud))
        std::cerr << "some points could not be added" << std::endl;


    ndt::renderNDTGrid(multigrid, Point(-10, -10), Point(10,10), display);
    cv::imshow("display", display);
    while(true) {
        int key = cv::waitKey(0) & 0xFF;
        if(key == 27)
            break;
    }

    return 0;
}