(require "tw.ss")

(clear)

(hint-normalise)

(light-diffuse 0 (vector 0 0 0))
(light-specular 0 (vector 0 0 0))
(light-ambient 0 (vector 0 0 0))
 
(define light (make-light 'directional 'free))
(light-specular light #(1 1 1))

; intialize the anttweakbar module
(tw-init)

; add a bar and iconify it
(define bar (tw-new-bar "TweakBar"))
(tw-define "TweakBar iconified=true")

; boxed variables for tw
(define zoom (box 1.0))
(define rotation (box (vector 0 0 0 1)))
(define auto-rotate (box #f))
(define light-multiplier (box 1.0))
(define light-dir (box (vector 0.69 -0.62 -0.38)))
(define mat-ambient (box (vector .5 0 0 1)))
(define mat-diffuse (box (vector 1 1 0 1)))

(define shape (box 'torus))

; add 'zoom' to 'bar', its key shortcuts are [z] and [Z]
(tw-add-var bar "Zoom" zoom
            "min=0.1 max=2.5 step=.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size)'.")

; add 'rotation' to 'bar': this is a variable of quaternion type
; which defines the object's orientation
(tw-add-quat bar "Rotation" rotation
             "label='Object rotation' open help='Change the object orientation.'")

; add variable to toggle auto-rotate mode
(tw-add-var bar "Auto-rotate" auto-rotate
            "label='Auto-rotate' key=space help='Toggle auto-rotate mode.'")

; add 'light-multiplier' to 'bar'
; its key shortcuts are [+] and [-].
(tw-add-var bar "Multiplier" light-multiplier
            "label='Light booster' min=0.1 max=4 step=0.02 keyIncr='+' keyDecr='-' help='Increase/decrease the light power.'")

; add 'light-dir' to 'bar'
; which defines the light direction
(tw-add-dir bar "LightDir" light-dir
            "label='Light direction' open help='Change the light direction.'")

; add 'mat-ambient' to 'bar': this is a variable of colour type
; and insert into a group called 'Material' 
(tw-add-colour bar "Ambient" mat-ambient "group='Material'")

; add 'mat-diffuse to 'bar': this is a variable of colour type
; and insert into a group called 'Material' 
(tw-add-colour bar "Diffuse" mat-diffuse "group='Material'")

; add the list variable 'shape' to 'bar'
; it's possible values are associated with the given list of symbols
(tw-add-list bar "Shape" shape '(torus cube cylinder)
             "keyIncr='<' keyDecr='>' help='Change object shape.'")

(every-frame
  (begin
    (light-direction light (unbox light-dir))
    (let ([ld (* (unbox light-multiplier) .8)]
          [la (* (unbox light-multiplier) .4)]) 
        (light-diffuse light (vector ld ld ld))
        (light-ambient light (vector la la la)))

    (scale (unbox zoom))

    (if (unbox auto-rotate)
      (begin
        (tw-define "TweakBar/Rotation readonly=true")
        (set-box! rotation (qmul (unbox rotation)
                                 (qaxisangle #(0 1 0) (* 50 (delta))))))
      (begin
        (tw-define "TweakBar/Rotation readonly=false")))
    (concat (qtomatrix (unbox rotation)))

    (ambient (unbox mat-ambient))
    (colour (unbox mat-diffuse))

    (case (unbox shape)
        ['torus (draw-torus)]
        ['cube (draw-cube)]
        ['cylinder (draw-cylinder)])))

