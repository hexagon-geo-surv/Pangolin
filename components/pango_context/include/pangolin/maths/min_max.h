// Copyright (c) farm-ng, inc. All rights reserved.

#pragma once

#include <pangolin/maths/eigen_numeric_limits.h>
#include <pangolin/maths/eigen_scalar_methods.h>
#include <sophus/image/image_view.h>

namespace pangolin
{

namespace
{
template <typename T>
concept Differencable = requires(T a, T b)
{
  b - a;
};
}  // namespace

// This works a lot like Eigens::AlignedBox, but supports
// TPixel as a scalar or vector element for use with generic
// image reductions etc
// The range of values is represented by min,max inclusive.
template <typename TPixel>
class MinMax
{
  public:
  MinMax() = default;
  MinMax(const MinMax&) = default;
  MinMax(MinMax&&) = default;
  MinMax& operator=(const MinMax&) = default;

  MinMax(const TPixel& p) : min_max{p, p} {}

  MinMax(const TPixel& p1, const TPixel& p2) : min_max{p1, p1} { extend(p2); }

  const TPixel& min() const { return min_max[0]; }
  const TPixel& max() const { return min_max[1]; }

  TPixel clamp(const TPixel& x) const
  {
    return pangolin::max(pangolin::min(x, min_max[1]), min_max[0]);
  }

  bool contains(const TPixel& x) const
  {
    return allTrue(eval(min() <= x)) && allTrue(eval(x <= max()));
  }

  auto fractionalPosition(const Eigen::Array2d& x) const
  {
    return eval(
        (x - pangolin::cast<double>(min())) / pangolin::cast<double>(range()));
  }

  // Only applicable if minmax object is valid()
  auto range() const requires Differencable<TPixel>
  {
    return eval(max() - min());
  }
  auto mid() const { return eval(min() + range() / 2); }

  MinMax<TPixel>& extend(const MinMax& o)
  {
    min_max[0] = pangolin::min(min(), o.min());
    min_max[1] = pangolin::max(max(), o.max());
    return *this;
  }

  MinMax<TPixel>& extend(const TPixel& p)
  {
    min_max[0] = pangolin::min(min(), p);
    min_max[1] = pangolin::max(max(), p);
    return *this;
  }

  MinMax<TPixel> translated(const TPixel& p) const
  {
    return {min_max[0] + p, min_max[1] + p};
  }

  template <typename To>
  MinMax<To> cast() const
  {
    return MinMax<To>(pangolin::cast<To>(min()), pangolin::cast<To>(max()));
  }

  bool empty() const
  {
    return min_max[0] == MultiDimLimits<TPixel>::max() ||
           min_max[1] == MultiDimLimits<TPixel>::lowest();
  }

  static MinMax<TPixel> open()
  {
    MinMax<TPixel> mm;
    mm.min_max = {MultiDimLimits<TPixel>::min(), MultiDimLimits<TPixel>::max()};
    return mm;
  }

  static MinMax<TPixel> closed()
  {
    MinMax<TPixel> mm;
    mm.min_max = {MultiDimLimits<TPixel>::max(), MultiDimLimits<TPixel>::min()};
    return mm;
  }

  private:
  // invariant that min_max[0] <= min_max[1]
  // or min_max is as below when uninitialized
  std::array<TPixel, 2> min_max = {
      MultiDimLimits<TPixel>::max(), MultiDimLimits<TPixel>::min()};
};

namespace details
{
template <class TScalar>
class Cast<MinMax<TScalar>>
{
  public:
  template <typename To>
  static MinMax<To> impl(const MinMax<TScalar>& v)
  {
    return v.template cast<To>();
  }
  template <typename To>
  static auto implScalar(const MinMax<TScalar>& v)
  {
    using ElT = decltype(cast<To>(std::declval<TScalar>()));
    return v.template cast<ElT>();
  }
};
}  // namespace details

template <typename T>
bool operator==(const MinMax<T>& lhs, const MinMax<T>& rhs)
{
  return lhs.min() == rhs.min() && lhs.max() == rhs.max();
}

template <typename T>
auto relative(T p, MinMax<T> region)
{
  return (p - region.min()).eval();
}

template <typename T>
auto normalized(T p, MinMax<T> region)
{
  return (cast<double>(p - region.min()) / cast<double>(region.range())).eval();
}

template <typename Tpixel>
MinMax<Tpixel> finiteMinMax(const sophus::ImageView<Tpixel>& image)
{
  return image.reduce(
      [](Tpixel v, auto& min_max) {
        if (isFinite(v)) min_max.extend(v);
      },
      MinMax<Tpixel>{});
}

template <typename Tpixel>
inline MinMax<Eigen::Vector2i> minMaxImageCoordsInclusive(
    const sophus::ImageView<Tpixel>& image, int border = 0)
{
  // e.g. 10x10 image has valid values [0, ..., 9] in both dimensions
  // a border of 2 would make valid range [2, ..., 7]
  return MinMax<Eigen::Vector2i>(Eigen::Vector2i(border, border))
      .extend(Eigen::Vector2i(
          image.width() - border - 1, image.height() - border - 1));
}

}  // namespace pangolin
