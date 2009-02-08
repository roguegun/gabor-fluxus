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

(provide find-missile-txts
		 texture-location)

(define texture-location "textures/")

;; returns missile texture filename strings for the given missile index
(define (find-missile-txts d)
  (map (lambda (f)
		 (path->string f))
		 (find-files (lambda (f)
				(regexp-match (pregexp (string-append "missile"
													  (number->string d)
													  "\\d\\.png$"))
				  (if (path? f)
					(path->string f)
					f)))
				texture-location)))
