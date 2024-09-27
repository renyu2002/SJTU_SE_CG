#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <png.h>

//support for libpng version > 14
#ifndef png_infopp_NULL
# define png_infopp_NULL (png_infopp)NULL
#endif
#ifndef png_voidp_NULL
# define png_voidp_NULL (png_voidp)NULL
#endif
#ifndef int_p_NULL
# define int_p_NULL (int*)NULL
#endif

struct imgInfo {
    int width;
    int height;
    png_byte *image_data;
};


// puts read data into img
// note: img.image_data should be freed after use.
void bytesof(const char *file_name, struct imgInfo *img) {

    // reset the struct, just to be safe
    img->width  = 0;
    img->height = 0;
    img->image_data = NULL;

    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0) {
        perror(file_name);
        return;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    // if (width){ *width = temp_width; }
    // if (height){ *height = temp_height; }
    img->width  = temp_width;
    img->height = temp_height;

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    // png_byte *image_data;
    img->image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (img->image_data == NULL) {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep *row_pointers = (png_bytep *)malloc(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL) {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        // free(image_data);
        fclose(fp);
        return;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    unsigned int i;
    for (i = 0; i < temp_height; i++) {
        row_pointers[temp_height - 1 - i] = img->image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // clean up
    // png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    // free(row_pointers);
    fclose(fp);
}

GLuint png_texture_load(const char *file_name, int *width, int *height) {

    struct imgInfo img;

    bytesof(file_name, &img);
    if (img.image_data == NULL) { // an error has occurred
        printf("image loading failed: %s\n", file_name);
        return 0;
    }

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // free(img.image_data);
    *width  = img.width;
    *height = img.height;

    return texture;
}

void loadFace(GLenum face, char *file_name) {

    struct imgInfo img;
    bytesof(file_name, &img);
    if (img.image_data == NULL) { // an error has occurred
        printf("cubemap face loading failed: %s\n", file_name);
        return;
    }

    glTexImage2D(face, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.image_data);
    free(img.image_data);
}

// base name will have things like "positive_x.png" added to it.
GLuint png_cubemap_load(const char *base_name) {

    GLuint texture;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // glEnable(GL_TEXTURE_CUBE_MAP);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    char path[200];
    loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, strcat(strcpy(path, base_name), "positive_x.png"));
    loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, strcat(strcpy(path, base_name), "negative_x.png"));
    loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, strcat(strcpy(path, base_name), "positive_y.png"));
    loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, strcat(strcpy(path, base_name), "negative_y.png"));
    loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, strcat(strcpy(path, base_name), "positive_z.png"));
    loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, strcat(strcpy(path, base_name), "negative_z.png"));

    // glDisable(GL_TEXTURE_CUBE_MAP);


    return texture;
}
