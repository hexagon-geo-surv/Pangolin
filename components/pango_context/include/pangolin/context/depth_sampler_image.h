#pragma once

#include <pangolin/context/depth_sampler.h>

namespace pangolin
{

class DepthSamplerImage : public DepthSampler
{
  public:
  virtual void setDepthImage(const sophus::Image<float>& image) = 0;

  struct Params {
    DepthKind kind = DepthKind::zaxis;
    sophus::Image<float> depth_image;
  };
  static Shared<DepthSamplerImage> Create(Params p);
};

}  // namespace pangolin
