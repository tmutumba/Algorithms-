#include <cinttypes>
#include <cmath>
#include <iostream>
#include <random>

template <typename T> constexpr auto MAX(T a, T b) -> decltype(a + b) {
  return (a > b) ? a : b;
}

template <typename T> constexpr auto MIN(T a, T b) -> decltype(a + b) {
  return (a < b) ? a : b;
}

constexpr int INT_RADIX_BASE = 10;

using unary_real_func = double (*)(double);

auto randrange(double min, double max) -> double {
  std::random_device rd{};
  std::mt19937 e(rd());
  std::uniform_real_distribution<> dist(0, 1);
  return min + dist(e) * (max - min);
}

// f(x) = sin(1/x)
auto fx(double x) -> double { return sin(1.0 / x); }

auto mc_avg_integrate(unary_real_func f, double a, double b,
                      int64_t samples) -> double {
  double sum = 0.0;
  for (int i = 0; i < samples; i++) {
    double x = randrange(a, b);
    sum += f(x);
  }
  return (b - a) * (sum / samples);
}

auto mc_bound_integrate(unary_real_func f, double a, double b,
                        int64_t samples) -> double {
  double min_y = HUGE_VAL_F64;
  double max_y = -HUGE_VAL_F64;
  double curr;
  double x;
  double y;
  int inside = 0;
  double delta = (b - a) / samples;
  for (int i = 0; i <= samples; i++) {
    curr = f(a + i * delta);
    max_y = MAX(max_y, curr);
    min_y = MIN(min_y, curr);
  }
  for (int i = 0; i < samples; i++) {
    y = randrange(min_y, max_y);
    x = randrange(a, b);
    curr = f(x);
    if ((y >= 0 && y <= curr) || (y <= 0 && y >= curr)) {
      inside++;
    }
  }
  return (b - a) * (max_y - min_y) * inside / samples;
}

auto trapezoid_integrate(unary_real_func f, double a, double b,
                         int64_t samples) -> double {
  double delta = (b - a) / samples;
  double area = 0;
  double last = f(a);
  double curr;
  for (int i = 1; i <= samples; i++) {
    curr = f(a + i * delta), area += (curr + last) * delta / 2.0,
    last = curr;
  }
  return area;
}

auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <trails>";
    exit(EXIT_FAILURE);
  }

  int64_t samples = strtol(argv[1], nullptr, INT_RADIX_BASE);
  std::cout
      << "Using " << samples
      << " samples for integration from 0 to 100 of f(x) = sin(1/x) by: "
      << std::endl;
  std::cout << "Expected is ~5.027962." << std::endl;
  std::cout << "Trapezoidal Rule: "
            << trapezoid_integrate(fx, 0.0, 100.0, samples) << std::endl;
  std::cout << "Monte-Carlo Mean values: "
            << mc_avg_integrate(fx, 0.0, 100.0, samples) << std::endl;
  std::cout << "Monte-Carlo Bounded samples: "
            << mc_bound_integrate(fx, 0.0, 100.0, samples) << std::endl;
}