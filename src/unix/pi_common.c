#include <unistd.h>
#include <string.h>

/* Could use Hardware DMA here */

void Com_Memcpy( void* dest, const void* src, const size_t count ) {
         memcpy( dest, src, count );
         }
void Com_Memset( void* dest, const int val, const size_t count ) {
         memset( dest, val, count );
         }

#include <GLES/gl.h>
extern void etc1_compress_tex_image(
   GLenum target,
   GLint level,
   GLenum internalformat,
   GLsizei width,
   GLsizei height,
   GLint border,
   GLenum format,
   GLenum type,
   const GLvoid *pixels);

static int isopaque(GLint width, GLint height, const GLvoid *pixels)
{
   unsigned char const *cpixels = (unsigned char const *)pixels;

   int i;

   for (i = 0; i < width * height; i++) {
      if (cpixels[i*4+3] != 0xff)
         return 0;
   }

   return 1;
}

void rgba4444_convert_tex_image(
   GLenum target,
   GLint level,
   GLenum internalformat,
   GLsizei width,
   GLsizei height,
   GLint border,
   GLenum format,
   GLenum type,
   const GLvoid *pixels)
{
   unsigned char const *cpixels = (unsigned char const *)pixels;

   unsigned short *rgba4444data = malloc(2 * width * height);

   int i;

   for (i = 0; i < width * height; i++) {
      unsigned char r,g,b,a;

      r = cpixels[4*i]>>4;
      g = cpixels[4*i+1]>>4;
      b = cpixels[4*i+2]>>4;
      a = cpixels[4*i+3]>>4;

      rgba4444data[i] = r << 12 | g << 8 | b << 4 | a;
   }

   glTexImage2D(target, level, format, width, height,border,format,GL_UNSIGNED_SHORT_4_4_4_4,rgba4444data);

   free(rgba4444data);
}

void myglTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   static int opaque = 0;

   if (format == GL_RGB ) {
      opaque = 1;
      etc1_compress_tex_image(target, level, format, width, height, border, format, type, pixels);
   } else if (format == GL_RGBA) {
      if (level == 0)
         opaque = isopaque(width, height, pixels);

      if (opaque)
         etc1_compress_tex_image(target, level, format, width, height, border, format, type, pixels);
      else
         rgba4444_convert_tex_image(target, level, format, width, height, border, format, type, pixels);
   }
}
