#pragma once

#include <pangolin/color/color.h>
#include <sophus/image/image_types.h>
#include <sophus/lie/isometry3.h>

// Specialization of sophus::ImageTraits for types we might like to consider
// in pixel or vertex-like buffers

namespace sophus
{

template <class TT>
struct ImageTraits<Rotation3<TT>> {
  // (X,Y,Z,W) quaternion
  static const int kNumChannels = 4;
  using TPixel = Rotation3<TT>;
  using ChannelT = TT;
};

template <class TT>
struct ImageTraits<Isometry3<TT>> {
  // (X,Y,Z,W) quaternion +
  // (tx,ty,tz) translation
  static const int kNumChannels = 7;
  using TPixel = Isometry3<TT>;
  using ChannelT = TT;
};

template <>
struct ImageTraits<pangolin::Color> {
  static const int kNumChannels = 4;
  using TPixel = pangolin::Color;
  using ChannelT = float;
};

}  // namespace sophus