(clear)

(define dither-vert
"
void main(void)
{
  gl_Position = ftransform();
  gl_TexCoord[0] = gl_MultiTexCoord0;
}")

(define dither-frag
"
uniform sampler2D color;
uniform vec2 tsize;

void main(void)
{
    const vec4 grey = vec4(.3, .59, .11, .0);

    float c = dot(texture2D(color, gl_TexCoord[0].xy), grey);
    vec2 xy = mod(gl_TexCoord[0].xy * tsize.xy, 4.);
    
    #undef D
    #define D 63.0
    mat4 dither0 = mat4(1, 9, 3, 11,
                    13, 5, 15, 7,
                    4, 12,  2, 10,
                    16, 8, 14, 6);
    mat4 dither = dither0 / D;
/*    
   mat4 dither = mat4(...) / D
   on osx results in:
   (0) : fatal error C9999: Non scalar or vector type in ConvertNamedConstantsExpr()
   Cg compiler terminated due to fatal error
*/
    if (c <= dither[int(xy.x)][int(xy.y)])
        gl_FragColor = vec4(0, 0, 0, 1);
    else
        gl_FragColor = vec4(1);
}")

(define p (build-pixels 256 256 #t))
(with-primitive p
    (scale 0))

(with-state
  (texture (pixels->texture p))
  (scale 8)
  (shader-source dither-vert dither-frag)
  (shader-set! #:tsize #(256 256)) 
  (build-cube))

(every-frame
    (with-pixels-renderer p
        (colour .5) 
        (identity)
        (rotate (vector (* 30 (time)) -45 37))
        (scale 3)
        (draw-cylinder)))

