(require racket/class)
(require "easing.ss")

(clear)

(set-camera-transform (mtranslate #(0 0 -10)))

(define (vvmul v0 v1)
  (build-vector (vector-length v0)
                (λ (i)
                   (* (vector-ref v0 i) (vector-ref v1 i)))))

(define (vvmix v0 v1 vt)
  (build-vector (vector-length v0)
                (λ (i)
                   (lerp (vector-ref v0 i) (vector-ref v1 i) (vector-ref vt i)))))

(define ease-box%
    (class object%
        (init-field name fn rect)

        (define p0 (vector-ref rect 0))
        (define p1 (vector-ref rect 1))

        (define sc (* .04 (- (vx p1) (vx p0))))

        (with-primitive (build-polygons 4 'quad-list)
            (pdata-index-map!
                (λ (i p)
                    (case i
                        [(0) p0]
                        [(1) (vector (vx p1) (vy p0) 0)]
                        [(2) p1]
                        [(3) (vector (vx p0) (vy p1) 0)]))
                "p")
            (hint-none)
            (hint-unlit)
            (hint-wire))

        (with-primitive (build-ribbon 64)
            (pdata-index-map!
                (λ (i p)
                    (vvmix p0 p1 (vector (/ i (- (pdata-size) 1))
                                         (fn (/ i (- (pdata-size) 1)))
                                         1)))
                "p")
            (hint-none)
            (hint-wire))

        (with-state
             (translate (vvmix p0 p1 #(.05 .85 0)))
             (scale .06)
             (build-type fluxus-scratchpad-font name))

        (define ball (with-state
                       (hint-unlit)
                       (hint-ignore-depth)
                       (colour #(1 0 0))
                       (build-sphere 3 3)))

        (define/public (update t)
            (with-primitive ball
                (identity)
                (translate (vvmix p0 p1 (vector t (fn t) 1)))
                (scale sc)))

        (super-new)))

(define ease-fns
    (list (cons "ease-none" ease-none)
          (cons "ease-in-quad" ease-in-quad)
          (cons "ease-out-quad" ease-out-quad)
          (cons "ease-in-out-quad" ease-in-out-quad)
          (cons "ease-out-in-quad" ease-out-in-quad)
          (cons "ease-in-cubic" ease-in-cubic)
          (cons "ease-out-cubic" ease-out-cubic)
          (cons "ease-in-out-cubic" ease-in-out-cubic)
          (cons "ease-out-in-cubic" ease-out-in-cubic)
          (cons "ease-in-quart" ease-in-quart)
          (cons "ease-out-quart" ease-out-quart)
          (cons "ease-in-out-quart" ease-in-out-quart)
          (cons "ease-out-in-quart" ease-out-in-quart)
          (cons "ease-in-quint" ease-in-quint)
          (cons "ease-out-quint" ease-out-quint)
          (cons "ease-in-out-quint" ease-in-out-quint)
          (cons "ease-out-in-quint" ease-out-in-quint)
          (cons "ease-in-sine" ease-in-sine)
          (cons "ease-out-sine" ease-out-sine)
          (cons "ease-in-out-sine" ease-in-out-sine)
          (cons "ease-out-in-sine" ease-out-in-sine)
          (cons "ease-in-expo" ease-in-expo)
          (cons "ease-out-expo" ease-out-expo)
          (cons "ease-in-out-expo" ease-in-out-expo)
          (cons "ease-out-in-expo" ease-out-in-expo)
          ))


(define (build-ease-boxes)
    (let* ([n (length ease-fns)]
           [n2 (add1 (inexact->exact (floor (sqrt n))))]
           [xsize (/ 19.8 n2)]
           [ysize (/ 14.8 n2)]
           [p #(-9.9 -7.4 0)]
           [step (vector xsize ysize 0)])

        (for/list ([efn ease-fns]
                   [i (in-range n)])
           (let-values ([(y x) (quotient/remainder i n2)])
               (make-object ease-box% (car efn) (cdr efn)
                                      (vector
                                          (vadd p (vvmul step (vector x y 0)))
                                          (vadd p (vvmul step (vector (+ x 1) (+ y 1) 0)))))))))

(define ease-boxes (build-ease-boxes))

(define (animate)
  (define t (clamp (* (fmod (* (time) .5) 1) 1.5) 0 1))
  (for-each
    (λ (eb)
      (send eb update t))
    ease-boxes))

(every-frame (animate))

