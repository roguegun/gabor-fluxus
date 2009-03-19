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

; generate 256 random particles
(define p (build-particles 256))

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

; generate and draw delaunay triangulated particles
(define (delaunay-particles)
	; convert particle coordinates to 2d list of points
    (define points
        (with-primitive p
            (for/list ([i (in-range (pdata-size))])
                (let ([p (pdata-ref "p" i)])
                    (vector (vx p) (vy p))))))
	; call triangulate to get the delaunay edges
    (define edges (triangulate points '(delaunay-edges)))
    
	; draw edges
	(for ([edge edges])
		(let ([line (build-ribbon 2)])
			(with-primitive line
				(pdata-set! "p" 0
					(with-primitive p
						(pdata-ref "p" (car edge))))
				(pdata-set! "p" 1
					(with-primitive p
						(pdata-ref "p" (cdr edge))))
				(hint-unlit)
				(hint-wire))
			line)))

(delaunay-particles)

