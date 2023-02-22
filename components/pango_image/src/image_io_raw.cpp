#include <pangolin/image/image_io.h>
#include <pangolin/utils/variant_overload.h>

#include <fstream>

using namespace sophus;

namespace pangolin
{

IntensityImage<> LoadImageNonPlanar(
    const std::string& filename, const PixelFormat& raw_fmt, size_t raw_width,
    size_t raw_height, size_t raw_pitch, size_t offset)
{
  ImageLayout shape = ImageLayout::makeFromSizeAndPitch<uint8_t>(
      ImageSize(raw_width, raw_height), raw_pitch);
  IntensityImage<> img(shape, raw_fmt);

  // Read from file, row at a time.
  std::ifstream bFile(filename.c_str(), std::ios::in | std::ios::binary);
  bFile.seekg(offset);
  for (int r = 0; r < img.height(); ++r) {
    bFile.read((char*)img.rawRowPtr(r), img.pitchBytes());
    if (bFile.fail()) {
      PANGO_WARN("Unable to read raw image file to completion.");
      break;
    }
  }
  return img;
}

template <typename Tin, typename Tout>
IntensityImage<> ToNonPlanarT(const IntensityImageView& planar_image)
{
  PixelFormat new_fmt = PixelFormat::fromTemplate<Tout>();
  const size_t planes = new_fmt.num_channels;

  PANGO_ENSURE(planar_image.height() % planes == 0);
  PANGO_ENSURE(sizeof(Tin) * planes == sizeof(Tout));
  PANGO_ENSURE(sizeof(Tout) == new_fmt.bytesPerPixel());

  ImageView<Tin> in = planar_image.imageView<Tin>();
  MutImage<Tout> out(
      {planar_image.width(), planar_image.height() / int(planes)});

  for (size_t c = 0; c < planes; ++c) {
    ImageView<Tin> in_plane =
        in.subview({0, out.height() * c}, out.imageSize());

    for (int y = 0; y < in_plane.height(); ++y) {
      Tin* p_out = (Tin*)out.rowPtr(y) + c;
      const Tin* p_in = in_plane.rowPtr(y);
      const Tin* p_end = p_in + in_plane.width();

      while (p_in != p_end) {
        *p_out = *p_in;

        ++p_in;
        p_out += planes;
      }
    }
  }

  return out;
}

IntensityImage<> ToNonPlanar(const IntensityImage<>& planar, size_t planes)
{
  IntensityImage<> ret;

  if (planes == 3) {
    visitImage(
        overload{
            [&](const ImageView<uint8_t>& image) {
              ret = ToNonPlanarT<uint8_t, Pixel3U8>(image);
            },
            [&](const ImageView<uint16_t>& image) {
              ret = ToNonPlanarT<uint16_t, Pixel3U16>(image);
            },
            [&](const ImageView<float>& image) {
              ret = ToNonPlanarT<float, Pixel3<float>>(image);
            },
            [&](const auto&) {
              PANGO_THROW(
                  "Unable to convert planar image of type {}",
                  planar.pixelFormat());
            },
        },
        planar);
  } else if (planes == 4) {
    visitImage(
        overload{
            [&](const ImageView<uint8_t>& image) {
              ret = ToNonPlanarT<uint8_t, Pixel4U8>(image);
            },
            [&](const ImageView<uint16_t>& image) {
              ret = ToNonPlanarT<uint16_t, Pixel4U16>(image);
            },
            [&](const ImageView<float>& image) {
              ret = ToNonPlanarT<float, Pixel4<float>>(image);
            },
            [&](const auto&) {
              PANGO_THROW(
                  "Unable to convert planar image of type {}",
                  planar.pixelFormat());
            },
        },
        planar);
  } else {
    PANGO_THROW("Unsupported number of image planes to output {}", planes);
  }

  return ret;
}

IntensityImage<> LoadImage(
    const std::string& filename, const PixelFormat& raw_plane_fmt,
    size_t raw_width, size_t raw_height, size_t raw_pitch, size_t offset,
    size_t image_planes)
{
  image_planes = std::max<size_t>(1, image_planes);

  if (image_planes > 1) {
    // Load as large image
    IntensityImage<> planar = LoadImageNonPlanar(
        filename, raw_plane_fmt, raw_width, raw_height * image_planes,
        raw_pitch, offset);
    return ToNonPlanar(planar, image_planes);
  } else {
    return LoadImageNonPlanar(
        filename, raw_plane_fmt, raw_width, raw_height, raw_pitch, offset);
  }
}

}  // namespace pangolin
