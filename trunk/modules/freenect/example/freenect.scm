(clear)

(require fluxus-017/freenect)

(printf "There are ~a Kinects connected~n" (freenect-get-num-devices))

(texture-params 0 '(wrap-s clamp wrap-t clamp))

(define kinect (freenect-open 0))

(freenect-set-depth-mode 'hist)

(hint-ignore-depth)
(hint-unlit)

(with-freenect-device kinect
    (displayln kinect)
    (freenect-set-tilt 0)
    (let ([p-rgb (build-plane)]
          [p-depth (build-plane)]
          [t-rgb (freenect-get-rgb-texture)]
          [t-depth (freenect-get-depth-texture)]
          [tcoords (freenect-tcoords)])
        (with-primitive p-rgb
            (texture t-rgb)
            (translate #(-.51 0 0))
            (pdata-index-map!
                (lambda (i t)
                    (list-ref tcoords (remainder i 4)))
                "t"))
        (with-primitive p-depth
            (texture t-depth)
            (opacity .5)
            (translate #(.51 0 0))
            (pdata-index-map!
                (lambda (i t)
                    (list-ref tcoords (remainder i 4)))
                "t"))))
 
(every-frame (with-freenect-device kinect
                (freenect-update)))

