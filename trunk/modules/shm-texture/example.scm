(require fluxus-017/shm-texture)

(clear)

(define t (shm-texture 5656 320 240 'greyscale))
(texture t)

(texture-params 0 '(wrap-s clamp wrap-t clamp))
(hint-wire)

(let ([p (build-plane)]
      [tcoords (shm-tcoords t)])
    (with-primitive p
        (scale #(4 3 1))
        (pdata-index-map!
            (lambda (i t)
                (list-ref tcoords (remainder i 4)))
            "t")))

(every-frame
    (shm-update t))
