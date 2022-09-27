#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/display/default_font.h>

extern const unsigned char AnonymousPro_ttf[];

int main( int /*argc*/, char** /*argv*/ )
{
    using namespace pangolin;

    CreateWindowAndBind("Main",640,480);
//    glEnable(GL_DEPTH_TEST);

    std::shared_ptr<GlFont> font;
    GlTexture font_offsets;

    font = std::make_shared<GlFont>(AnonymousPro_ttf, 32, 1024, 1024, false);
    font->InitialiseGlTexture();
    font_offsets.Load(font->MakeFontLookupImage());
    font_offsets.SetNearestNeighbour();

    GlSlProgram prog;
    auto reload_shader = [&](){
        const std::string shader_path = "~/code/Pangolin/examples/HelloPangolin/test.glsl";
        prog.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader_path, {}, {});
        prog.BindPangolinDefaultAttribLocationsAndLink();
        prog.Bind();
        prog.SetUniform("u_font_atlas", 0);
        prog.SetUniform("u_font_offsets", 1);
    };
    reload_shader();


    pangolin::GlBuffer vbo;
    GlVertexArrayObject vao;
    std::vector<Eigen::Vector3f> host_vbo = {
       {0.0,0.0,0.0},
    };

    vbo = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo );
    vao.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo);
    vao.Unbind();

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DisplayBase().Activate();

        prog.Bind();
        glActiveTexture(GL_TEXTURE0);
        font->mTex.Bind();
        glActiveTexture(GL_TEXTURE1);
        font_offsets.Bind();
        glDrawArrays(GL_POINTS, 0, vbo.num_elements);
        prog.Unbind();

        pangolin::FinishFrame();
    }

    return 0;
}
