#lang racket
;; Easing functions based on libcinder's easing.
;; https://github.com/cinder/Cinder/blob/master/include/cinder/Easing.h

(provide (all-defined-out))

;; Easing equation for a simple linear tweening with no easing.
(define (ease-none t)
  t)

;; Easing equation for a quadratic (t^2) ease-in, accelerating from zero velocity.
(define (ease-in-quad t)
  (* t t))

;; Easing equation for a quadratic (t^2) ease-out, decelerating to zero velocity.
(define (ease-out-quad t)
  (* (- t) (- t 2)))


;; Easing equation for a quadratic (t^2) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-quad t)
  (define t2 (* t 2))
  (if (< t2 1)
    (* .5 t2 t2)
    (let ([t3 (sub1 t2)])
      (* -.5 (- (* t3 (- t3 2)) 1)))))

;; Easing equation for a quadratic (t^2) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-quad t)
  (if (< t .5)
	(* .5 (ease-out-quad (* 2 t)))
	(+ .5 (* .5 (ease-in-quad (- (* 2 t) 1))))))


;; Easing equation function for a cubic (t^3) ease-in, accelerating from zero velocity.
(define (ease-in-cubic t)
  (* t t t))

;; Easing equation for a cubic (t^3) ease-out, decelerating to zero velocity.
(define (ease-out-cubic t)
  (define t2 (- t 1))
  (+ (* t2 t2 t2) 1))

; Easing equation for a cubic (t^3) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-cubic t)
  (define t2 (* t 2))
  (if (< t2 1)
	(* .5 t2 t2 t2)
	(let ([t3 (- t2 2)])
	  (* .5 (+ (* t3 t3 t3) 2)))))

;; Easing equation for a cubic (t^3) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-cubic t)
  (if (< t .5)
	(/ (ease-out-cubic (* 2 t)) 2)
	(+ (/ (ease-in-cubic (- (* 2 t) 1)) 2) .5)))

;; Easing equation for a quartic (t^4) ease-in, accelerating from zero velocity.
(define (ease-in-quart t)
  (* t t t t))

;; Easing equation for a quartic (t^4) ease-out, decelerating to zero velocity.
(define (ease-out-quart t)
  (define t2 (- t 1))
  (- (- (* t2 t2 t2 t2) 1)))

;; Easing equation for a quartic (t^4) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-quart t)
  (define t2 (* t 2))
  (if (< t2 1)
	(* .5 t2 t2 t2 t2)
	(let ([t3 (- t2 2)])
	  (* -.5 (- (* t3 t3 t3 t3) 2)))))

;; Easing equation for a quartic (t^4) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-quart t)
  (if (< t .5)
	(/ (ease-out-quart (* 2 t)) 2)
	(+ (/ (ease-in-quart (- (* 2 t) 1)) 2) .5)))

;; Easing equation function for a quintic (t^5) ease-in, accelerating from zero velocity.
(define (ease-in-quint t)
  (* t t t t t))

;; Easing equation for a quintic (t^5) ease-out, decelerating to zero velocity.
(define (ease-out-quint t)
  (define t2 (- t 1))
  (+ (* t2 t2 t2 t2 t2) 1))

;; Easing equation for a quintic (t^5) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-quint t)
  (define t2 (* t 2))
  (if (< t2 1)
	(* .5 t2 t2 t2 t2 t2)
	(let ([t3 (- t2 2)])
	  (* .5 (+ (* t3 t3 t3 t3 t3) 2)))))

;; Easing equation for a quintic (t^5) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-quint t)
  (if (< t .5)
	(/ (ease-out-quint (* 2 t)) 2)
	(+ (/ (ease-in-quint (- (* 2 t) 1)) 2) .5)))

;; Easing equation for a sinusoidal (sin(t)) ease-in, accelerating from zero velocity.
(define (ease-in-sine t)
  (+ (- (cos (* t (/ pi 2)))) 1))

;; Easing equation for a sinusoidal (sin(t)) ease-out, decelerating from zero velocity.
(define (ease-out-sine t)
  (sin (* t (/ pi 2))))

;; Easing equation for a sinusoidal (sin(t)) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-sine t)
  (* -.5 (- (cos (* t pi)) 1)))

;; Easing equation for a sinusoidal (sin(t)) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-sine t)
  (if (< t .5)
	(/ (ease-out-sine (* 2 t)) 2)
	(+ (/ (ease-in-sine (- (* 2 t ) 1)) 2) .5)))

;; Easing equation for an exponential (2^t) ease-in, accelerating from zero velocity.
(define (ease-in-expo t)
  (if (zero? t)
	0
	(expt 2 (* 10 (- t 1)))))

;; Easing equation for an exponential (2^t) ease-out, decelerating from zero velocity.
(define (ease-out-expo t)
  (if (= t 1)
	1
	(- 1 (expt 2 (* -10 t)))))

;; Easing equation for an exponential (2^t) ease-in/out, accelerating until halfway, then decelerating.
(define (ease-in-out-expo t)
  (cond [(zero? t) 0]
		[(= t 1) 1]
		[else
		  (define t2 (* t 2))
		  (if (< t2 1)
			(* .5 (expt 2 (* 10 (- t2 1))))
			(* .5 (- 2 (expt 2 (* -10 (- t2 1))))))]))

;; Easing equation for an exponential (2^t) ease-out/in, decelerating until halfway, then accelerating.
(define (ease-out-in-expo t)
  (if (< t .5)
	(/ (ease-out-expo (* 2 t)) 2)
	(+ (/ (ease-in-expo (- (* 2 t) 1)) 2) .5)))


