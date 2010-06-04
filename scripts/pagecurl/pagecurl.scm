(require scheme/set)

(clear)

(hint-ignore-depth)

; hack lock-camera
(set-camera-transform (mtranslate #(0 0 -10)))

(define-values (w h) (vector->values (get-screen-size)))

; page resolution
(define-values (pw ph) (values (/ w 2) h))

; set 2d ortho projection
(ortho)
(set-ortho-zoom 1)
(frustum w 0 0 h)

(define imm-prims (set)) ; set of primitives that live for one frame only

;; destroy all one-frame primitives
(define (imm-destroy)
    (set-for-each
        imm-prims
        destroy)
    (set! imm-prims (set)))

;; add primitive to one-frame primitives
(define (imm-add p)
  (set! imm-prims (set-add imm-prims p)))

;; debug primitives

(define (draw-line x0 y0 x1 y1)
  (let ([p (build-ribbon 2)])
    (with-primitive p
                    (pdata-set! "p" 0 (vector x0 y0 0))
                    (pdata-set! "p" 1 (vector x1 y1 0))
                    (hint-wire)
                    (hint-unlit))
    (imm-add p)))

(define (draw-point x y)
    (with-state
        (hint-unlit)
        (translate (vector x y 0))
        (scale 5)
        (draw-sphere)))

;; clamp coordinates for page flipping
(define (clamp-coords x y)
    (cond [(and (< x 0) (>= y ph)) ; below bottom-left corner
                (values 0 ph)]
          [(and (>= x (* 2 pw)) (>= y ph)) ; below bottom-right corner
                (values (* 2 pw) ph)]
    
          [(< y ph) ; above bottom line
                (let* ([dx (- x pw)]
                       [dy (- y ph)]
                       [d (sqrt (+ (* dx dx) (* dy dy)))]
                       [dmax pw])
                    (if (< d dmax)
                        (values x y)
                        (values (+ pw (* dmax (/ dx d)))
                                (+ ph (* dmax (/ dy d))))))]
          [else ; below bottom line
                (let* ([dx (- x pw)]
                       [dy y]
                       [d (sqrt (+ (* dx dx) (* dy dy)))]
                       [dmax (sqrt (+ (* pw pw) (* ph ph)))])
                    (if (< d dmax)
                        (values x y)
                        (values (+ pw (* dmax (/ dx d)))
                                (* dmax (/ dy d)))))]))

;; mirror point (px, py) to line (x0, y0), (x1, y1)
(define (mirror-point px py x0 y0 x1 y1)
  (let* ([v0 (- px x0)] ; vector of p from x0
         [v1 (- py y0)]
         [u0 (- x1 x0)] ; line direction vector
         [u1 (- y1 y0)]
         [s (/ (+ (* v0 u0) (* v1 u1)) (+ (* u0 u0) (* u1 u1)))]
         [px1 (+ x0 (* s u0))] ; p projected to line
         [py1 (+ y0 (* s u1))])
    (vector (- (* 2 px1) px) (- (* 2 py1) py) 0)))

;; returns the points and uv coordinates of the page's curled part:
;; x, y - position of the page's bottom left corner 
;; returns 2 values: points uvs
;; the first two points define the slant of the fold
(define (page-curl-points x y)
    (with-handlers ([exn:fail:contract:divide-by-zero?
                (lambda (exn) ; page is not visible
                      (values 
                          (list (vector 0 0 0)
                                (vector 0 ph 0))
                          (list (vector 1 1 0)
                                (vector 1 0 0))))
                    ])
        (let*-values ([(x y) (clamp-coords x y)]
                      [(px py) (values 0 ph)] ; page corner
                      [(cx cy) (values (/ (+ px x) 2) (/ (+ py y) 2))]
                      [(vx vy) (values (- x px) (- y py))]
                      [(sqv) (values (sqrt (+ (* vx vx) (* vy vy))))]
                      [(nx ny) (values (/ vy sqv) (/ (- vx) sqv))] ; bisection normal
                      [(y0 xpy x0) (values (- cy (* ny (/ cx nx))) ; bisection intersection at x=0
                                           (- cx (* nx (/ (- cy py) ny))) ; bisection intersection at y=py
                                           (- cx (* nx (/ cy ny))))] ; y=0
                )
            (let ([points (if (> x0 0)
                            (list (vector x0 0 0)
                                  (vector xpy py 0)
                                  (vector x y 0)
                                  (mirror-point 0 0 cx cy (+ cx nx) (+ cy ny)))
                            (list (vector 0 y0 0)
                                  (vector xpy py 0)
                                  (vector x y 0)))]
                  [uvs (if (> x0 0)
                         (list (vector (- 1 (/ x0 pw)) 1 0)
                               (vector (- 1 (/ xpy pw)) 0 0)
                               (vector 1 0 0)
                               (vector 1 1 0))
                         (list (vector 1 (- 1 (/ y0 ph)) 0)
                               (vector (- 1 (/ xpy pw)) 0 0)
                               (vector 1 0 0)))])
              (values points uvs))

            #|
            (draw-line x y px py)
            (with-state
                (colour #(1 0 0))
                (draw-point cx cy))
            (with-state
                (hint-wire-stippled)
                (draw-line cx cy (+ cx (* 50 nx)) (+ cy (* 50 ny))))
            (with-state
                (colour #(0 1 0))
                (draw-point 0 y0)
                (draw-point x0 0)
                (draw-point xpy py))
            |#
            )))

;; load book pages
(require scheme/file)
(require srfi/13)

(texture-params 0 '(wrap-s clamp wrap-t clamp))

;; returns a list of texture handles for the png files in the
;; given directory
(define (load-textures dir)
    (map
        (lambda (imgp)
            (load-texture (path->string imgp)))
        (find-files
            (lambda (x)
                (let ([s (path->string x)])
                    (string-suffix? ".png" s)))
            (string->path dir))))

(define book (load-textures "book"))

(define page0-shadow (load-texture "gfx/flipGrad.png"))

;; draw pages
;; i - currenly flipped page index
(define (page-curl x y i)
  (with-state
    (hint-unlit)
    (colour 1)
    (let*-values ([(points uvs) (page-curl-points x y)]
                  [(flip-vx) (values 
                               (lambda (v)
                                 (vector (- 1 (vx v)) (vy v) (vz v))))])

                 ; draw pages in display order
                 ; page - frontside of flipped page
                 (let ([page (build-polygons (- 8 (length points)) 'polygon)])
                   (with-primitive page
                       (texture (list-ref book i))
                       (cond [(= (pdata-size) 4)
                              (pdata-set! "p" 0 (list-ref points 0))
                              (pdata-set! "p" 1 (list-ref points 1))
                              (pdata-set! "p" 2 (vector pw ph 0))
                              (pdata-set! "p" 3 (vector pw 0 0))

                              (pdata-set! "t" 0 (flip-vx (list-ref uvs 0)))
                              (pdata-set! "t" 1 (flip-vx (list-ref uvs 1)))
                              (pdata-set! "t" 2 (vector 1 0 0))
                              (pdata-set! "t" 3 (vector 1 1 0))
                              ]
                             [else ; pdata-size = 5
                               (pdata-set! "p" 0 (vector 0 0 0))
                               (pdata-set! "p" 1 (list-ref points 0))
                               (pdata-set! "p" 2 (list-ref points 1))
                               (pdata-set! "p" 3 (vector pw ph 0))
                               (pdata-set! "p" 4 (vector pw 0 0))

                               (pdata-set! "t" 0 (vector 0 1 0))
                               (pdata-set! "t" 1 (flip-vx (list-ref uvs 0)))
                               (pdata-set! "t" 2 (flip-vx (list-ref uvs 1)))
                               (pdata-set! "t" 3 (vector 1 0 0))
                               (pdata-set! "t" 4 (vector 1 1 0)) ]))
                   (imm-add page))

                 ; page1 - next page after flipped one
                 (let ([page1 (build-polygons 4 'quad-list)])
                   (with-primitive page1
                       (texture (if (< i (sub1 (length book)))
                                  (list-ref book (+ i 1))
                                  0))
                       (pdata-set! "p" 0 (vector pw 0 0))
                       (pdata-set! "p" 1 (vector pw ph 0))
                       (pdata-set! "p" 2 (vector (* 2 pw) ph 0))
                       (pdata-set! "p" 3 (vector (* 2 pw) 0 0))

                       (pdata-set! "t" 0 (vector 0 1 0))
                       (pdata-set! "t" 1 (vector 0 0 0))
                       (pdata-set! "t" 2 (vector 1 0 0))
                       (pdata-set! "t" 3 (vector 1 1 0)))
                   (imm-add page1))

                 ; page -1 - page below flipped page
                 (let ([page-1 (build-polygons (length points) 'polygon)])
                   (when (> (length points) 2)
                     (with-primitive page-1
                         (texture (if (>= i 2)
                                    (list-ref book (- i 2))
                                    0))
                         (cond [(= (pdata-size) 3)
                                (pdata-set! "p" 0 (list-ref points 0))
                                (pdata-set! "p" 1 (vector 0 ph 0))
                                (pdata-set! "p" 2 (list-ref points 1))

                                (pdata-set! "t" 0 (flip-vx (list-ref uvs 0)))
                                (pdata-set! "t" 1 (vector 0 0 0))
                                (pdata-set! "t" 2 (flip-vx (list-ref uvs 1)))]
                               [else
                                 (pdata-set! "p" 0 (vector 0 0 0))
                                 (pdata-set! "p" 1 (vector 0 ph 0))
                                 (pdata-set! "p" 2 (list-ref points 1))
                                 (pdata-set! "p" 3 (list-ref points 0))

                                 (pdata-set! "t" 0 (vector 0 1 0))
                                 (pdata-set! "t" 1 (vector 0 0 0))
                                 (pdata-set! "t" 2 (flip-vx (list-ref uvs 1)))
                                 (pdata-set! "t" 3 (flip-vx (list-ref uvs 0)))])))
                   (imm-add page-1))

                 ; page 0 - backside of flipped page
                 (let ([page0 (build-polygons (length points) 'polygon)])
                   (when (> (length points) 2)
                     (with-primitive page0
                         (texture (if (>= i 1)
                                    (list-ref book (- i 1))
                                    0))
                         (pdata-index-map!
                           (lambda (i p)
                             (list-ref points i))
                           "p")
                         (pdata-index-map!
                           (lambda (i p)
                             (list-ref uvs i))
                           "t")))
                   (imm-add page0))
                 )))

(define (animate)
    (imm-destroy)
    (draw-point (mouse-x) (mouse-y))
    (page-curl (mouse-x) (mouse-y) 3))

(every-frame (animate))

