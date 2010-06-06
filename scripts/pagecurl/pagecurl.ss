#lang scheme

(require fluxus-017/fluxus)
(require scheme/math)
(require scheme/set)

(provide
  pc-init
  pc-update
  pc-next
  pc-prev
  pc-load)

(define-values (w h) (vector->values (get-screen-size)))

; page resolution
(define-values (pw ph) (values (/ w 2) h))
(define margin 50)

(define book '())

(define (pc-init)
	(clear)

	(hint-ignore-depth)

	; hack lock-camera
	(set-camera-transform (mtranslate #(0 0 -10)))

	; set 2d ortho projection
	(ortho)
	(set-ortho-zoom 1)

	(set! book '())
	(frustum (+ w margin) (- margin) (- margin) (+ h margin)))

;; load book pages
(require scheme/file)
(require srfi/13)

;; loads a list of texture handles for the png files in the
;; given directory
(define (pc-load dir)
    (set! book (map
					(lambda (imgp)
						(load-texture (path->string imgp)))
					(find-files
						(lambda (x)
							(let ([s (path->string x)])
								(string-suffix? ".png" s)))
						(string->path dir)))))


;; loads all sounds from dir
(define (load-sounds dir)
    (map
		(lambda (sndp)
			(oa-load-sample (path->string sndp)))
		(find-files
			(lambda (x)
				(let ([s (path->string x)])
					(string-suffix? ".wav" s)))
			(string->path dir))))

(oa-start)
(define sounds (load-sounds "sfx"))

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
;; returns 4 values: clamped-x clamped-y points uvs
;; the first two points define the slant of the fold
(define (page-curl-points x y)
    (define-values (clx cly) (clamp-coords x y))
    (with-handlers ([exn:fail:contract:divide-by-zero?
                (lambda (exn) ; page is not visible
                      (values 
                          clx cly
                          (list (vector 0 0 0)
                                (vector 0 ph 0))
                          (list (vector 1 1 0)
                                (vector 1 0 0))))
                    ])
        (let*-values ([(px py) (values 0 ph)] ; page corner
                      [(cx cy) (values (/ (+ px clx) 2) (/ (+ py cly) 2))]
                      [(vx vy) (values (- clx px) (- cly py))]
                      [(sqv) (values (sqrt (+ (* vx vx) (* vy vy))))]
                      [(nx ny) (values (/ vy sqv) (/ (- vx) sqv))] ; bisection normal
                      [(y0 xpy x0) (values (- cy (* ny (/ cx nx))) ; bisection intersection at x=0
                                           (- cx (* nx (/ (- cy py) ny))) ; bisection intersection at y=py
                                           (- cx (* nx (/ cy ny))))] ; y=0
                )
            (let ([points (if (> x0 0)
                            (list (vector x0 0 0)
                                  (vector xpy py 0)
                                  (vector clx cly 0)
                                  (mirror-point 0 0 cx cy (+ cx nx) (+ cy ny)))
                            (list (vector 0 y0 0)
                                  (vector xpy py 0)
                                  (vector clx cly 0)))]
                  [uvs (if (> x0 0)
                         (list (vector (- 1 (/ x0 pw)) 1 0)
                               (vector (- 1 (/ xpy pw)) 0 0)
                               (vector 1 0 0)
                               (vector 1 1 0))
                         (list (vector 1 (- 1 (/ y0 ph)) 0)
                               (vector (- 1 (/ xpy pw)) 0 0)
                               (vector 1 0 0)))])
              (values clx cly points uvs))

            #|
            (draw-line clx cly px py)
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

(define page0-shadow (load-texture "gfx/page0-sh.png"))
(define page-1-shadow (load-texture "gfx/page-1-sh.png"))
(define page-shadow (load-texture "gfx/page-sh.png"))

;; draw pages
;; i - currenly flipped page index
;; corner - page corner ('bottom-left 'bottom-right)
(define (page-curl x y i corner)
  (with-state
    (backfacecull 0)
    (hint-unlit)
    (colour 1)
    (cond [(eq? corner 'bottom-right)
               (set! x (- w x))
               (translate (vector (/ w 2) (/ h 2) 0))
               (scale #(-1 1 1))
               (translate (vector (- (/ w 2)) (- (/ h 2)) 0))])

    (let* ([flip-vx (lambda (v)    ; flip vector in x
                       (vector (- 1 (vx v)) (vy v) (vz v)))]

          ; set texture coords according to direction
          [pdata-t-set! (lambda (i v)
                          (cond [(eq? corner 'bottom-right)
                                     (pdata-set! "t" i (vector (- 1 (vx v)) (vy v) (vz v)))]
                                [else
                                      (pdata-set! "t" i v)]))]

          ; projects point p onto line p0, p1
          ; and returns scaling factor
          [project-pnt-scale (lambda (p p0 p1)
                                  (let* ([v0 (- (vx p) (vx p0))] ; vector of p from p0
                                         [v1 (- (vy p) (vy p0))]
                                         [u0 (- (vx p1) (vx p0))] ; line direction vector
                                         [u1 (- (vy p1) (vy p0))])
                                    (/ (+ (* v0 u0) (* v1 u1)) (+ (* u0 u0) (* u1 u1)))))]
          ; point line distance
          [pnt-line-distance (lambda (p p0 p1)
                                  (let* ([u0 (- (vx p1) (vx p0))] ; line direction vector
                                         [u1 (- (vy p1) (vy p0))]
                                         [s (project-pnt-scale p p0 p1)]
                                         [px1 (+ (vx p0) (* s u0))] ; p projected to line
                                         [py1 (+ (vy p0) (* s u1))])
                                    (sqrt (+ (sqr (- px1 (vx p)))
                                             (sqr (- py1 (vy p)))))))]
          ; point line vector
          [pnt-line-vector (lambda (p p0 p1)
                                  (let* ([u0 (- (vx p1) (vx p0))] ; line direction vector
                                         [u1 (- (vy p1) (vy p0))]
                                         [s (project-pnt-scale p p0 p1)]
                                         [px1 (+ (vx p0) (* s u0))] ; p projected to line
                                         [py1 (+ (vy p0) (* s u1))])
                                    (vector (- (vx p) px1) (- (vy p) py1) 0)))]

          ; page textures
          [page-1-txt    (with-handlers ([exn:fail:contract?
                                          (lambda (exn) 0)])
                                (if (eq? corner 'bottom-left)
                                     (list-ref book (- i 2))
                                     (list-ref book (+ i 2))))]
          [page0-txt    (with-handlers ([exn:fail:contract?
                                          (lambda (exn) 0)])
                                (if (eq? corner 'bottom-left)
                                     (list-ref book (- i 1))
                                     (list-ref book (+ i 1))))]
          [page-txt    (with-handlers ([exn:fail:contract?
                                          (lambda (exn) 0)])
                                (if (eq? corner 'bottom-left)
                                     (list-ref book (+ i 1))
                                     (list-ref book (- i 1))))]
          )

      (define-values (clamp-x clamp-y points uvs) (page-curl-points x y))
      (define shadow-opac (let ([d (vdist (vector clamp-x clamp-y 0)
                                          (vector (* 2 pw) ph 0))]
                                [md (/ pw 4)])
                            (if (> d md)
                                  1
                                (- 1 (/ (- md d) md)))))

      ; draw pages in display order
      ; page1 - next page after flipped one
      ; TODO: shadows would be much easier to add if fluxus supported stencil buffer
      (let ([page1 (build-polygons 4 'quad-list)])
        (with-primitive page1
                        (texture page-txt)
                        (when (zero? page-txt)
                          (colour 0))
                        (pdata-set! "p" 0 (vector pw 0 0))
                        (pdata-set! "p" 1 (vector pw ph 0))
                        (pdata-set! "p" 2 (vector (* 2 pw) ph 0))
                        (pdata-set! "p" 3 (vector (* 2 pw) 0 0))

                        (pdata-t-set! 0 (vector 0 1 0))
                        (pdata-t-set! 1 (vector 0 0 0))
                        (pdata-t-set! 2 (vector 1 0 0))
                        (pdata-t-set! 3 (vector 1 1 0)))
        (imm-add page1))

      ; page - frontside of flipped page
      (let* ([page (build-polygons (- 8 (length points)) 'polygon)]
             [sh (build-polygons 4 'quad-list)])
        (with-primitive page
                        (texture (list-ref book i))
                        (cond [(= (pdata-size) 4)
                               (pdata-set! "p" 0 (list-ref points 0))
                               (pdata-set! "p" 1 (list-ref points 1))
                               (pdata-set! "p" 2 (vector pw ph 0))
                               (pdata-set! "p" 3 (vector pw 0 0))

                               (pdata-t-set! 0 (flip-vx (list-ref uvs 0)))
                               (pdata-t-set! 1 (flip-vx (list-ref uvs 1)))
                               (pdata-t-set! 2 (vector 1 0 0))
                               (pdata-t-set! 3 (vector 1 1 0))
                               ]
                              [else ; pdata-size = 5
                                (pdata-set! "p" 0 (vector 0 0 0))
                                (pdata-set! "p" 1 (list-ref points 0))
                                (pdata-set! "p" 2 (list-ref points 1))
                                (pdata-set! "p" 3 (vector pw ph 0))
                                (pdata-set! "p" 4 (vector pw 0 0))

                                (pdata-t-set! 0 (vector 0 1 0))
                                (pdata-t-set! 1 (flip-vx (list-ref uvs 0)))
                                (pdata-t-set! 2 (flip-vx (list-ref uvs 1)))
                                (pdata-t-set! 3 (vector 1 0 0))
                                (pdata-t-set! 4 (vector 1 1 0)) ]))
        (when (> (length points) 2)
          (with-primitive sh
                        (texture page-shadow) 
                        (let* ([p0 (list-ref points 0)]
                               [p1 (list-ref points 1)]
                               [vd (vsub p1 p0)]
                               [nd (pnt-line-vector (list-ref points 2) p0 p1)])

                               (pdata-set! "p" 0 (vsub p0 vd))
                               (pdata-set! "p" 1 (vadd p1 vd))
                               (pdata-set! "p" 2 (vadd (pdata-ref "p" 1) nd))
                               (pdata-set! "p" 3 (vadd (pdata-ref "p" 0) nd))
                               
                               ; texture blending artefacts around vertical borders
                               (pdata-set! "t" 0 #(0.03 1 0))
                               (pdata-set! "t" 1 #(0.03 0 0))
                               (pdata-set! "t" 2 #(.97 0 0))
                               (pdata-set! "t" 3 #(.97 1 0))
                          )))
        (imm-add page)
        (imm-add sh))

      ; page -1 - page below flipped page
      (let ([page-1 (build-polygons (length points) 'polygon)]
            [sh (build-polygons (length points) 'polygon)])
        (when (> (length points) 2)
          (with-primitive page-1
                          (texture page-1-txt)
                          (when (zero? page-1-txt)
                                (colour 0))
                          (cond [(= (pdata-size) 3)
                                 (pdata-set! "p" 0 (list-ref points 0))
                                 (pdata-set! "p" 1 (vector 0 ph 0))
                                 (pdata-set! "p" 2 (list-ref points 1))

                                 (pdata-t-set! 0 (flip-vx (list-ref uvs 0)))
                                 (pdata-t-set! 1 (vector 0 0 0))
                                 (pdata-t-set! 2 (flip-vx (list-ref uvs 1)))]
                                [else
                                  (pdata-set! "p" 0 (vector 0 0 0))
                                  (pdata-set! "p" 1 (vector 0 ph 0))
                                  (pdata-set! "p" 2 (list-ref points 1))
                                  (pdata-set! "p" 3 (list-ref points 0))

                                  (pdata-t-set! 0 (vector 0 1 0))
                                  (pdata-t-set! 1 (vector 0 0 0))
                                  (pdata-t-set! 2 (flip-vx (list-ref uvs 1)))
                                  (pdata-t-set! 3 (flip-vx (list-ref uvs 0)))]))
          (with-primitive sh
              (texture page-1-shadow)
              (opacity shadow-opac)
              (pdata-index-map!
                (lambda (i p)
                      (with-primitive page-1
                            (pdata-ref "p" i)))
                "p")

              (cond [(= (pdata-size) 3)
                     (pdata-set! "t" 0 (vector 1 1 0))
                     (pdata-set! "t" 1 (vector 0 (project-pnt-scale (pdata-ref "p" 1)
                                                                    (pdata-ref "p" 0)
                                                                    (pdata-ref "p" 2)) 0))
                     (pdata-set! "t" 2 (vector 1 0 0))
                     ]
                    [else
                          (let ([s2 (project-pnt-scale (pdata-ref "p" 1)
                                                       (pdata-ref "p" 3)
                                                       (pdata-ref "p" 2))]
                                [s3 (project-pnt-scale (pdata-ref "p" 0)
                                                       (pdata-ref "p" 3)
                                                       (pdata-ref "p" 2))]
                                [d2 (pnt-line-distance (pdata-ref "p" 1)
                                                       (pdata-ref "p" 3)
                                                       (pdata-ref "p" 2))]
                                [d3 (pnt-line-distance (pdata-ref "p" 0)
                                                       (pdata-ref "p" 3)
                                                       (pdata-ref "p" 2))]
                                )
                              ;(printf "s3:~a s2:~a~n" s3 s2)
                              ;(printf "d3:~a d2:~a~n" d3 d2)
                              (cond [(> d2 d3)
                                          ;(printf "1 - d3/d2: ~a~n" (- 1 (/ d3 d2)))
                                          (pdata-set! "t" 0 (vector (- 1 (/ d3 d2)) 1 0))
                                          (pdata-set! "t" 1 (vector 0 s2 0))
                                          (pdata-set! "t" 2 (vector 1 0 0))
                                          (pdata-set! "t" 3 (vector 1 (+ 1 s3) 0))
                                          ]
                                    [else
                                          (pdata-set! "t" 0 (vector 0 s3 0))
                                          (pdata-set! "t" 1 (vector (- 1 (/ d2 d3)) 0 0))
                                          (pdata-set! "t" 2 (vector 1 (- 1 s2) 0))
                                          (pdata-set! "t" 3 (vector 1 1 0))]))
                          ])
          )
        (imm-add page-1)
        (imm-add sh)))

      ; page 0 - backside of flipped page
      (let ([page0 (build-polygons (length points) 'polygon)]
            [sh (build-polygons (length points) 'polygon)])
        (when (> (length points) 2)
          (with-primitive page0
                          (texture page0-txt)
                          (when (zero? page0-txt)
                            (colour 0))
                          (pdata-index-map!
                            (lambda (i p)
                              (list-ref points i))
                            "p")
                          (for ([i (in-range (pdata-size))])
                               (pdata-t-set! i (list-ref uvs i))))
          (with-primitive sh
                          (texture page0-shadow)
                          (opacity shadow-opac)
                          (pdata-index-map!
                            (lambda (i p)
                              (list-ref points i))
                            "p")
                          (cond [(= (pdata-size) 3)
                                 (pdata-set! "t" 0 (vector 1 1 0))
                                 (pdata-set! "t" 1 (vector 1 0 0))
                                 (pdata-set! "t" 2 (vector 0 (project-pnt-scale (list-ref points 2)
                                                                                (list-ref points 0)
                                                                                (list-ref points 1)) 0))
                                 ]
                                [else
                                  (let ([s2 (project-pnt-scale (list-ref points 2)
                                                               (list-ref points 0)
                                                               (list-ref points 1))]
                                        [s3 (project-pnt-scale (list-ref points 3)
                                                               (list-ref points 0)
                                                               (list-ref points 1))]
                                        [d2 (pnt-line-distance (list-ref points 2)
                                                               (list-ref points 0)
                                                               (list-ref points 1))]
                                        [d3 (pnt-line-distance (list-ref points 3)
                                                               (list-ref points 0)
                                                               (list-ref points 1))]
                                        )
                                      ;(printf "s3:~a s2:~a~n" s3 s2)
                                      ;(printf "d3:~a d2:~a~n" d3 d2)
                                      (cond [(> d2 d3)
                                                  ;(printf "1 - d3/d2: ~a~n" (- 1 (/ d3 d2)))
                                                  (pdata-set! "t" 0 (vector 1 (+ 1 s3) 0))
                                                  (pdata-set! "t" 1 (vector 1 0 0))
                                                  (pdata-set! "t" 2 (vector 0 (- 1 s2) 0))
                                                  (pdata-set! "t" 3 (vector (- 1 (/ d3 d2)) 1 0))
                                                  ]
                                            [else
                                                  (pdata-set! "t" 0 (vector 1 1 0))
                                                  (pdata-set! "t" 1 (vector 1 (- s2 1) 0))
                                                  (pdata-set! "t" 2 (vector (- 1 (/ d2 d3)) 0 0))
                                                  (pdata-set! "t" 3 (vector 0 s3 0))]))

                                  ])))
                        
        (imm-add page0)
        (imm-add sh))
      )))

(define (move-corner t)
    (hermite (vector -1 ph 0)
             (vector (add1 (* 2 pw)) ph 0) ; FIXME: display bug at (2pw, ph)
             (vector 400 -400 0)
             (vector 400 550 0) t))

(define pc-state 'idle) ; one of '(idle next prev)
(define pc-t 1.0)
(define pc-page 0)
(define pc-dir 'bottom-right) ; one of '(bottom-left bottom-right)
(define pc-speed 1.5)

(define (pc-next)
    (when (and (eq? pc-state 'idle)
               (< pc-page (- (length book) 1)))
        (set! pc-state 'next)
		(oa-play (list-ref sounds (random (length sounds))) #(0 0 0) .9 1)))

(define (pc-prev)
    (when (and (eq? pc-state 'idle)
               (> pc-page 0))
        (set! pc-state 'prev)
		(oa-play (list-ref sounds (random (length sounds))) #(0 0 0) .9 1)))

(define (pc-update)
    (imm-destroy)

    (cond [(eq? pc-state 'next)
                (when (eq? pc-dir 'bottom-left)
                    (set! pc-dir 'bottom-right)
                    (set! pc-t 1)
                    (set! pc-page (add1 pc-page)))
                (if (> pc-t 0)
                    (set! pc-t (- pc-t (* pc-speed (delta))))
                    (begin
                        (set! pc-state 'idle)
                        (set! pc-t 0)
                        (set! pc-page (add1 pc-page))
                        (set! pc-dir 'bottom-left)))]
          [(eq? pc-state 'prev)
                (when (eq? pc-dir 'bottom-right)
                    (set! pc-dir 'bottom-left)
                    (set! pc-t 0)
                    (set! pc-page (sub1 pc-page)))
                (if (< pc-t 1)
                    (set! pc-t (+ pc-t (* (delta) pc-speed)))
                    (begin
                        (set! pc-state 'idle)
                        (set! pc-t 1)
                        (set! pc-page (sub1 pc-page))
                        (set! pc-dir 'bottom-right)))])

    (let ([v (move-corner pc-t)])
        ;(draw-point (vx v) (vy v))
        (page-curl (vx v) (vy v) pc-page pc-dir)))

