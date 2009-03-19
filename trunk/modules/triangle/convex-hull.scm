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

(define p (build-particles 256))

(with-primitive p
    (pdata-map!
        (lambda (p)
              (let ([cv (vmul (srndvec) 5)])
              (vector (vx cv) (vy cv) 0)))
        "p")
    (pdata-map!
        (lambda (c)
          1)
        "c"))

(define (convex-hull-particles)
    (define points
        (with-primitive p
            (for/list ([i (in-range (pdata-size))])
                (let ([p (pdata-ref "p" i)])
                    (vector (vx p) (vy p))))))
    (define convex-hull (triangulate points '(convex-hull)))
    
    (with-primitive p
        ; paint all particles white
        (pdata-map!
            (lambda (c)
                1)
            "c")
        (pdata-map!
            (lambda (c)
                .1)
            "s")
        ; paint convex hull particles red
        (for ([i convex-hull])
             (pdata-set! "c" i (vector 1 0 0))
             (pdata-set! "s" i .2))))

(convex-hull-particles)

