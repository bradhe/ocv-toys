#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>

#define WINDOW_NAME "Object Tracking"

int gcd(int a, int b) {
  if(b == 0) return a;
  return gcd(b, a % b);
}

double luminance(cv::Vec3b vec) {
  return (vec[0] * 0.11) + (vec[1] * 0.59) + (vec[2] * 0.30);
}

class SubMatrix
    : public cv::Mat {

  public:
    static const int DEFAULT_WIDTH  = 50;
    static const int DEFAULT_HEIGHT = 50;

    SubMatrix(cv::Mat& _mat, int offsetX, int offsetY)
      : cv::Mat(_mat.colRange(offsetX, offsetX + DEFAULT_WIDTH).rowRange(offsetY, offsetY + DEFAULT_HEIGHT)) { };

    SubMatrix(cv::Mat& _mat, int offsetX, int offsetY, int width, int height)
      : cv::Mat(_mat.colRange(offsetX, offsetX + width).rowRange(offsetY, offsetY + height)) { };

    double sum() {
      double sum=0;

      cv::MatConstIterator_<cv::Vec3b> it = this->begin<cv::Vec3b>(), end = this->end<cv::Vec3b>();

      for(; it != end; ++it) {
        sum += luminance(*it);
      }

      return(sum);
    };
};

class IntensitySearch {
  public:
    IntensitySearch(cv::Mat& _mat)
      : mat(_mat) {
    };

    cv::Point FindGreatestIntensity() {
      cv::Size size = mat.size();
      return FindGreatestIntensity(gcd(size.width, size.height));
    }

    cv::Point FindGreatestIntensity(int gcd) {
      cv::Size size = mat.size();
      double* keys = new double[(size.width / gcd) * (size.height / gcd)];

      for(int i = 0; i < size.width / gcd; i++) {
        for(int j = 0; j < size.height / gcd; j++) {
          SubMatrix sub(mat, i * gcd, j * gcd, gcd, gcd);
          keys[i+j] = sub.sum();
        }
      }

      int maxX = 0, maxY = 0;
      double maxSum = -1.0;

      for(int i = 0; i < (size.width / gcd); i++) {
        for(int j = 0; j < (size.height / gcd); j++) {
          if(maxSum < keys[i+j]) {
            maxSum = keys[i+j];
            maxX = i;
            maxY = j;
          }
        }
      }

      std::cout << "Greatest: " << maxX << ", " << maxY << std::endl;
      return cv::Point(maxX * gcd, maxY * gcd);
    }

  private:
    cv::Mat& mat;
};

class Square {
  public:
    Square(cv::Mat& _mat)
      : mat(_mat) { };

    ~Square() { };

    void Draw(int x, int y, int width, int height) {
      int halfWidth = width / 2;
      int halfHeight = height / 2;
      cv::Size size = mat.size();

      if((x - halfWidth) >= 0) {
        cv::Mat left = mat.col(x - halfWidth).rowRange(y - halfHeight, y + halfHeight);
        this->oneify(left);
      }

      if((x + halfWidth) <= (size.width - 1)) {
        cv::Mat right  = mat.col(x + halfWidth).rowRange(y - halfHeight, y + halfHeight);
        this->oneify(right);
      }

      if((y - halfHeight) >= 0) {
        cv::Mat top = mat.row(y - halfWidth).colRange(x - halfWidth, x + halfWidth);
        this->oneify(top);
      }

      if((y + halfHeight) <= (size.height - 1)) {
        cv::Mat bottom = mat.row(y + halfWidth).colRange(x - halfWidth, x + halfWidth);
        this->oneify(bottom);
      }
    };

  private:
    cv::Mat& mat;

    void oneify(cv::Mat& _mat) {
      cv::MatIterator_<cv::Vec3b> it = _mat.begin<cv::Vec3b>();
      cv::MatIterator_<cv::Vec3b> end = _mat.end<cv::Vec3b>();

      for(; it != end; ++it) {
        (*it)[0] = 0x00; // r
        (*it)[1] = 0xff; // g
        (*it)[3] = 0x00; // b
      }
    }
};

int main(int argc, char ** argv) {
  cv::namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
  cv::moveWindow(WINDOW_NAME, 100, 100);

  cv::VideoCapture cap(0);
  cap.set(CV_CAP_PROP_FORMAT, CV_8UC3);

  if(!cap.isOpened()) {
    std::cerr << "Video device not opened." << std::endl;
    return(0);
  }

  cv::Mat img;
  Square square(img);
  IntensitySearch searcher(img);

  // We will keep the GCD for later.
  int _gcd = -1;

  for(;;) {
    cap >> img;
    cv::Size size = img.size();

    if(_gcd < 1) {
      _gcd = gcd(size.width, size.height);

      // If it's STILL < 1, we've got a problemo.
      if(_gcd < 1) {
        throw std::exception();
      }

      // Subdivide this further.
      _gcd /= 4;
    }


    // Find the most intense spot and put a box there.
    cv::Point intensestPoint = searcher.FindGreatestIntensity(_gcd);
    square.Draw(intensestPoint.x + (_gcd / 2), intensestPoint.y + (_gcd / 2), _gcd, _gcd);

    cv::imshow(WINDOW_NAME, img);
  }
}
