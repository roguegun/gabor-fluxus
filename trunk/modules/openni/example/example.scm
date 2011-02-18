(require fluxus-017/fluxus-openni)

(clear)

(texture-params 0 '(wrap-s clamp wrap-t clamp))

(define depth (openni-depth-texture))

(let ([p (build-plane)]
      [tcoords (openni-tcoords)])
    (with-primitive p
        (scale #(20 15 1))
        (hint-unlit)
        (texture (openni-depth-texture))
        (pdata-index-map!
            (lambda (i t)
                (list-ref tcoords (remainder i 4)))
            "t")))


(define (mainloop)
    (openni-update))

(every-frame (mainloop))
