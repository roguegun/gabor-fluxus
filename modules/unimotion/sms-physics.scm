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

(require "unimotion.ss")

(define type (detect-sms))

(clear)

(collisions 1)
(set-max-physical 100)

(define plane
    (ground-plane (vector 0 1 0) -3))

(for* ([x (in-range -2 2 1.2)]
        [z (in-range -2 2 1.2)])
    (let ([c (build-cube)])
        (with-primitive c
            (translate (vector x -2.5 z)))
        (active-box c)))

(define (update)
    (let ([g (vmul (read-sms-real type) 2)])
        (gravity (vector (- (vx g)) -1 (vy g)))))

(every-frame (update))

