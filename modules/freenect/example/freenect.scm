(require fluxus-017/fluxus-freenect)

(printf "There are ~a Kinects connected~n" (freenect-get-num-devices))

(define kinect (freenect-open 0))

(displayln kinect)

