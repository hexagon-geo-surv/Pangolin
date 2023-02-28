#pragma once

#include <pangolin/layer/draw_layer.h>
#include <pangolin/layer/layer.h>
#include <pangolin/var/var.h>

namespace pangolin
{

// Forward declarations
struct Widget;

////////////////////////////////////////////////////////////////////
/// Supports displaying a 2D tweak-var style interface
///
struct WidgetLayer : public Layer {
  struct Params {
    std::string name = "";
    Size size_hint = {Parts{1}, Parts{1}};

    // Font to use for widget text. Empty will use the built-in font.
    std::string font_path = "";

    // Widget Unit Height. All widgets will be a multiple of this.
    int widget_height_pix = 50;

    // Tune the widget appearance
    float fraction_padding = 0.12;
    float fraction_fontheight = 0.4;
  };
  static Shared<WidgetLayer> Create(Params p);
};

template <typename T>
struct LayerConversionTraits<Var<T>> {
  static Shared<WidgetLayer> makeLayer(const Var<T>& var)
  {
    return WidgetLayer::Create(
        {.name = var.Meta().full_name, .size_hint = {Parts{1}, Pixels{50}}});
  }
};

}  // namespace pangolin
