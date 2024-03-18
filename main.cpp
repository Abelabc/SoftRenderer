#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

//光路径向量
Vec3f light_dir(1,1,1);
//眼睛postion
Vec3f       eye(0,-1,3);
//center
Vec3f    center(0,0,0);
//摄像机y轴向量
Vec3f        up(0,1,0);

struct GouraudShader : public IShader {
    //存储每个顶点的光照强度
    Vec3f varying_intensity = {0.,0.,0.}; // written by vertex shader, read by fragment shader

    //这个方法会从模型中读取一个顶点，然后将其从世界坐标系转换成屏幕坐标系。同时，它会计算出模型表面的光照强度。这个强度是模型表面法线和光源方向的点积，用于确定在给定点的光照强度。
    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        return gl_Vertex;
    }

    //这个方法根据上一步计算的光照强度和当前像素的重心坐标来确定像素的颜色。然后，它返回false，意味着我们不会丢弃这个像素。
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
};
struct Shader : public IShader {
    Vec3f          varying_intensity; // written by vertex shader, read by fragment shader
    //这是一个2×3的浮点数矩阵，用于保存uv坐标。
    mat<2,3,float> varying_uv;        // same as above

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        //给定一个UV坐标，返回该UV坐标对应初始纹理图像中的颜色。
        color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};
int main(int argc, char** argv) {
    //读取obj
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    //构造了一个模型视图矩阵"ModelView"
    lookat(eye, center, up);
    //视口变换正好将其变换成实际的屏幕或离屏缓冲（像素缓冲）坐标
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    //遍历模型的所有三角形和每个三角形的栅格化。
    GouraudShader shader;
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
