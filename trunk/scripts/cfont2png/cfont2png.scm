;; Copyright (C) GPLv3 2011 Gabor Papp
;;
;; converting aalib c fonts to png's

(clear)

(define (read-file-str filename)
  (call-with-input-file filename
    (lambda (in)
        (letrec ([loop (lambda (str)
                            (define line (read-line in))
                            (if (eof-object? line)
                                str
                                (loop (string-append str line))))])
            (loop "")))))

(define (bwimg->list filename)
  (let* ([str (read-file-str filename)]
         [nums (regexp-match* #rx"0x[0-9a-fA-F]." str)])
    (map (lambda (s)
           (string->number (substring s 2 4) 16))
        nums)))


;#|
(define font-chars (bwimg->list "font8.c"))
(define font-width 8)
(define font-height 8)
(define output "font8x8s.png")
;|#

#|
(define font-chars (bwimg->list "font14.c"))
(define font-width 8)
(define font-height 14)
(define output "font8x14.png")
|#

(define (get-char n)
  (let* ([char-size (* font-width font-height 1/8)]
         [offset (* n char-size)])
      (for/list ([i (in-range offset (+ offset char-size))])
         (for/list ([j (in-range (sub1 font-width) -1 -1)])
            (if (bitwise-bit-set? (list-ref font-chars i) j)
                  1
                0)))))

;; 256x1 font texture
(define p (build-pixels (* font-width 256) font-height))

(with-primitive p
    (scale (vector (* (/ font-width font-height) 256) 1 1))
    (let ([scanline (* font-width 256)])
        (for ([i (in-range 256)])
            (let* ([c (get-char i)]
                   [offset (* i font-width)])
               (for* ([x (in-range font-width)]
                      [y (in-range font-height)])
                    (pdata-set! "c" (+ offset (* y scanline) x)
                                (list-ref (list-ref c (- font-height 1 y)) x))))))
    (pixels-upload))

#|
;; 16x16 font texture
(define p (build-pixels (* font-width 16) (* font-height 16)))

(with-primitive p
    (let* ([char-size (* font-width font-height)]
           [scanline (* font-width 16)]
           [fontline (* scanline font-height)])
        (for ([i (in-range 256)])
            (let* ([yi (- 15 (quotient i 16))]
                   [xi (remainder i 16)]
                   [c (get-char i)]
                   [offset (+ (* yi fontline) (* xi font-width))])
               (for* ([x (in-range font-width)]
                      [y (in-range font-height)])
                    (pdata-set! "c" (+ offset (* y scanline) x)
                                (list-ref (list-ref c (- font-height 1 y)) x))))))
    (pixels-upload))
|#

(with-primitive p
    (save-primitive output))

