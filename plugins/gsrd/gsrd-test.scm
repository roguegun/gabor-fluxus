(clear)

(define s 512)

(scale 15)
(define p (build-pixels s s #t 2))

(define plugin (ffgl-load "gsrd" s s))

(with-ffgl plugin
    (displayln (ffgl-get-parameters))
    (ffgl-process p
        (pixels->texture p 1)
        (pixels->texture p 0)))

(with-primitive p
    (pixels-render-to (pixels->texture p 0))
    (pixels-display (pixels->texture p 1)))

(with-pixels-renderer p
    (with-state
        (texture (load-texture "transp.png"))
        (hint-unlit)
        (scale #(20 15 1))
        (build-plane)))

(every-frame
    (with-ffgl plugin
        (ffgl-set-parameter! #:ru .28
                             #:rv .02
                             #:k 0.047
                             #:f .1
                             #:iterations .5
                             #:reset (if (mouse-button 1) 1 0))))

