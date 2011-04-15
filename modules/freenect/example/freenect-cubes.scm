(clear)

(require fluxus-017/freenect)

(define kinect (freenect-open 0))

(freenect-set-depth-mode 'scaled)

(define thr .56)

(define (render)
    (translate #(0 0 -40))
    (with-freenect-device kinect
        (for* ([x (in-range 0 640 10)]
               [y (in-range 0 480 10)])
            (let ([z (freenect-depth-at x y)])
                (when (> z .56)
                    (with-state
                        (translate (vector (* (- x 320) .1) (* (- 240 y) .1) (* 100.0 (- z thr))))
                        (scale .4)
                        (colour (hsv->rgb (vector z .9 .6)))
                        (draw-cube)))))
        (freenect-update)))

(every-frame (render))
    