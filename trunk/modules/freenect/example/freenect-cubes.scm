(require racket/vector)

(clear)

(require fluxus-018/freenect)

(define kinect (freenect-open 0))

(freenect-set-depth-mode 'raw)

(define (render)
    (with-freenect-device kinect
        (for* ([x (in-range 0 640 10)]
               [y (in-range 0 480 10)])
            (let ([pos (freenect-worldcoord-at x y)])
                (when (> (vz pos) 0)
                    (with-state
                        (translate (vector-map * pos #(1 1 -4)))
                        (scale .1)
                        (colour (hsv->rgb (vector (* (vz pos) .1) .9 .6)))
                        (draw-cube)))))
        (freenect-update)))

(every-frame (render))
    