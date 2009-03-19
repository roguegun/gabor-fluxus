;; Copyright (C) 2009 Gabor Papp
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program. If not, see http://www.gnu.org/licenses/.

(clear)

(require "triangle.ss")

; generate random particles
(define p (build-particles 128))

(with-primitive p
    (pdata-map!
        (lambda (p)
              (let ([cv (vmul (crndvec) 5)])
				  (vector (vx cv) (vy cv) 0)))
        "p")
    (pdata-map!
        (lambda (c)
          1)
        "c"))

; generate and draw voronoi edges
(define (voronoi-edges)
	; convert particle coordinates to 2d list of points
    (define input-points
        (with-primitive p
            (for/list ([i (in-range (pdata-size))])
                (let ([p (pdata-ref "p" i)])
                    (vector (vx p) (vy p))))))
	; call triangulate to get the voronoi points and edges
    (define-values (vpoints vedges) (triangulate input-points '(voronoi-points voronoi-edges)))
    
	; draw voronoi edges
	(for ([edge vedges])
		(let ([line (build-ribbon 2)]
			  [i0 (car edge)]
			  [i1 (cdr edge)])
			(with-primitive line
				(pdata-set! "p" 0
						(let ([v2d (list-ref vpoints i0)])
						  (vector (vx v2d) (vy v2d) 0)))
				(pdata-set! "p" 1
					; infinite ray?
					(if (pair? i1)
						  (vadd (let ([v2d (list-ref vpoints i0)])
							  (vector (vx v2d) (vy v2d) 0))
							  (vmul (vector (car i1) (cdr i1) 0) 100))
						(let ([v2d (list-ref vpoints i1)])
						  (vector (vx v2d) (vy v2d) 0))))
				(hint-unlit)
				(hint-wire))
			line)))

(voronoi-edges)

