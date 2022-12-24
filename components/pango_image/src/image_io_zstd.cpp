
#include <pangolin/image/runtime_image.h>

#include <fstream>
#include <memory>

#ifdef HAVE_ZSTD
#include <zstd.h>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"

namespace pangolin
{

#pragma pack(push, 1)
struct zstd_image_header {
  char magic[4];
  char fmt[16];
  size_t w, h;
};
#pragma pack(pop)

void SaveZstd(
    const IntensityImage<>& image, std::ostream& out, int compression_level)
{
#ifdef HAVE_ZSTD
  // Write out header, uncompressed
  zstd_image_header header;
  memcpy(header.magic, "ZSTD", 4);
#if (defined(__GNUC__) && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#pragma GCC diagnostic pop
#endif
  strncpy(header.fmt, ToString(image.pixelType()).c_str(), sizeof(header.fmt));
  header.w = image.width();
  header.h = image.height();
  out.write((char*)&header, sizeof(header));

  // Write out image data
  const size_t output_buffer_size = ZSTD_CStreamOutSize();
  std::unique_ptr<char[]> output_buffer(new char[output_buffer_size]);

  ZSTD_CStream* const cstream = ZSTD_createCStream();
  if (cstream == nullptr) {
    throw std::runtime_error("ZSTD_createCStream() error");
  }

  const size_t initResult = ZSTD_initCStream(cstream, compression_level);
  PANGO_THROW_IF(
      ZSTD_isError(initResult), "ZSTD_initCStream() error : {}",
      ZSTD_getErrorName(initResult));

  const size_t row_size_bytes =
      image.numChannels() * image.numBytesPerPixelChannel() * image.width();

  for (int y = 0; y < image.height(); ++y) {
    ZSTD_inBuffer input = {image.rawRowPtr(y), row_size_bytes, 0};

    while (input.pos < input.size) {
      ZSTD_outBuffer output = {output_buffer.get(), output_buffer_size, 0};
      size_t left_to_read = ZSTD_compressStream(cstream, &output, &input);
      PANGO_THROW_IF(
          ZSTD_isError(left_to_read), "ZSTD_compressStream() error : %",
          ZSTD_getErrorName(left_to_read));
      out.write(output_buffer.get(), output.pos);
    }
  }

  ZSTD_outBuffer output = {output_buffer.get(), output_buffer_size, 0};
  const size_t remainingToFlush =
      ZSTD_endStream(cstream, &output); /* close frame */
  if (remainingToFlush) {
    throw std::runtime_error("not fully flushed");
  }
  out.write(output_buffer.get(), output.pos);

  ZSTD_freeCStream(cstream);
#else
  PANGOLIN_UNUSED(image);
  PANGOLIN_UNUSED(fmt);
  PANGOLIN_UNUSED(out);
  PANGOLIN_UNUSED(compression_level);
  throw std::runtime_error("Rebuild Pangolin for ZSTD support.");
#endif  // HAVE_ZSTD
}

IntensityImage<> LoadZstd(std::istream& in)
{
#ifdef HAVE_ZSTD
  // Read in header, uncompressed
  zstd_image_header header;
  in.read((char*)&header, sizeof(header));

  IntensityImage<> img(
      ImageSize(header.w, header.h), PixelFormatFromString(header.fmt));

  const size_t input_buffer_size = ZSTD_DStreamInSize();
  std::unique_ptr<char[]> input_buffer(new char[input_buffer_size]);

  ZSTD_DStream* dstream = ZSTD_createDStream();
  if (!dstream) {
    throw std::runtime_error("ZSTD_createDStream() error");
  }

  size_t read_size_hint = ZSTD_initDStream(dstream);
  PANGO_THROW_IF(
      ZSTD_isError(read_size_hint), "ZSTD_initDStream() error : % \n",
      ZSTD_getErrorName(read_size_hint));

  // Image represents our fixed buffer.
  ZSTD_outBuffer output = {
      const_cast<uint8_t*>(img.rawPtr()), img.sizeBytes(), 0};

  while (read_size_hint) {
    in.read(input_buffer.get(), read_size_hint);
    ZSTD_inBuffer input = {input_buffer.get(), read_size_hint, 0};
    while (input.pos < input.size) {
      read_size_hint = ZSTD_decompressStream(dstream, &output, &input);
      PANGO_THROW_IF(
          ZSTD_isError(read_size_hint), "ZSTD_decompressStream() error : %",
          ZSTD_getErrorName(read_size_hint));
    }
  }

  ZSTD_freeDStream(dstream);

  return img;
#else
  PANGOLIN_UNUSED(in);
  throw std::runtime_error("Rebuild Pangolin for ZSTD support.");
#endif  // HAVE_ZSTD
}

}  // namespace pangolin

#pragma GCC diagnostic pop
