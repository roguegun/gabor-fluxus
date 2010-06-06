(require "pagecurl.ss")

(pc-init)
(pc-load "book")

(define (update)
  (when (key-special-pressed 100)
    (pc-prev))
  (when (key-special-pressed 102)
    (pc-next))
  (pc-update))

(every-frame (update))

