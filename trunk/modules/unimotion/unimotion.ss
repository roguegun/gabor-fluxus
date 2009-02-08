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

#lang scheme

(require scheme/foreign)

(unsafe!)

(provide detect-sms
        read-sms-real)

(define libunimotion (ffi-lib "libUniMotion"))

(define _mac-types 
    (_enum '(unknown = 0
             powerbook
             ibook
             high-res-powerbook
             macbook)))

(define detect-sms
    (get-ffi-obj "detect_sms" libunimotion (_fun -> _mac-types))) 

(define read-sms-real
    (get-ffi-obj "read_sms_real" libunimotion
        (_fun (t : _mac-types)
            (x : (_ptr o _double))
            (y : (_ptr o _double))
            (z : (_ptr o _double))
            -> _int
            -> (vector x y z))))

