(require fluxus-017/freenect)

(printf "There are ~a Kinects connected~n" (freenect-get-num-devices))

(define kinect (freenect-open 0))

(with-freenect-device kinect
    (displayln kinect)
    (freenect-set-tilt 0))

