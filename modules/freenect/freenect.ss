#lang racket/base

(require fluxus-018/fluxus-freenect)
(provide
  (except-out (all-from-out fluxus-018/fluxus-freenect)
			  freenect-grab-device
			  freenect-ungrab-device)
  with-freenect-device)

;; StartFunctionDoc-en
;; with-freenect-device freenect-deviceid expression ...
;; Returns: result of last expression
;; Description:
;; Allows you to work with the specified freenect devices.
;; Example:
;; EndFunctionDoc

(define-syntax with-freenect-device
  (syntax-rules ()
    ((_ a b ...)
     (begin
       (freenect-grab-device a)
       (let ((r (begin b ...)))
         (freenect-ungrab-device)
         r)))))
