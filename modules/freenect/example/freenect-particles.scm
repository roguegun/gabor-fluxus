(clear)

(require fluxus-017/freenect)

(define kinect (freenect-open 0))

(define pw 640)
(define ph 480)

(define p (build-particles (* pw ph)))

(freenect-set-depth-mode 'hist)

(define particles-vert
"
attribute vec3 t;
uniform sampler2D depth_tex;
varying float depth;

void main(void)
{
    gl_TexCoord[0] = vec4(t, 0);

    depth = texture2D(depth_tex, t.st).b;
    vec4 vertex = vec4(1. - t.st, depth, 1.0);
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}")

(define particles-frag
"
varying float depth;

void main(void)
{
    gl_FragColor = vec4(vec3(depth), 1.0);
}")

(with-primitive p
    (hint-none)
    (hint-points)
    (scale 7)
    (translate #(-.5 -.5 0))
    (with-freenect-device kinect
        (texture (freenect-get-depth-texture)))
    (pdata-add "t" "v")
    (pdata-index-map!
      (lambda (i t)
        (let-values ([(y x) (quotient/remainder i pw)])
            (vector (/ x pw) (/ y ph) 0)))
      "t")
    (shader-source particles-vert particles-frag)
    (shader-set! #:depth_tex 0))

(every-frame
    (with-freenect-device kinect
        (freenect-update)))

