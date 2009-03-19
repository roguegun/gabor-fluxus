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

(hint-unlit)

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

; generate and draw voronoi cells
(define (voronoi-cells)
	; convert particle coordinates to 2d list of points
    (define input-points
        (with-primitive p
            (for/list ([i (in-range (pdata-size))])
                (let ([p (pdata-ref "p" i)])
                    (vector (vx p) (vy p))))))
	; call triangulate to get the voronoi cells
    (define-values (vpoints vcells)
        (triangulate input-points
            '(voronoi-points voronoi-cells)))
    
	; draw voronoi cells
	(for ([cell vcells]
		  [ci (in-range (length vcells))]
		  #:when (list? cell)) ; filter out #f infinite cells
		(let ([poly (build-polygons (length cell) 'triangle-fan)])
			(with-primitive poly
				(for ([pi cell]
					  [i (in-range (length cell))])
					(pdata-set! "p" i
						(let ([v2d (list-ref vpoints pi)])
							(vector (vx v2d) (vy v2d) 0))))
				(colour (rndvec))
			poly))))

(voronoi-cells)

