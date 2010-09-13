;; AntTweakBar GUI Library bindings for fluxus
;; Copyright (C) 2010 Gabor Papp
;;
;; AntTweakBar
;; Copyright (C) 2005-2009 Philippe Decaudin
;; http://www.antisphere.com/Wiki/tools:anttweakbar
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

#lang racket

(require ffi/unsafe)
(require (except-in fluxus-017/fluxus 
					fluxus-reshape-callback
					fluxus-input-callback
					fluxus-input-release-callback))

(provide tw-add-button
		 tw-add-separator
		 tw-init
		 tw-new-bar
		 tw-define
		 tw-add-var
		 tw-add-dir
		 tw-add-colour
		 tw-add-quat
		 tw-add-list
		 tw-save-config
		 tw-load-config
		 fluxus-reshape-callback
		 fluxus-input-callback
		 fluxus-input-release-callback)

(define libtw (ffi-lib "libAntTweakBar"))

;; override fluxus callbacks
(define-values (width height) (vector->values (get-screen-size)))

(define (fluxus-reshape-callback x y)
  (set! width x)
  (set! height y)
  (reshape x y)
  (tw-window-size x y))

(define mouse-wheel-pos 0)

(define (fluxus-input-callback key button special state x y mod)
  ;(printf "button: ~a state: ~a x: ~a y: ~a~n" button state x y)
  (let ([handled
		  (cond [(<= 3 button 4)
				   (set! mouse-wheel-pos
				      (if (= button 3)
						(add1 mouse-wheel-pos)
						(sub1 mouse-wheel-pos)))
				   (tw-mouse-wheel mouse-wheel-pos)]
				[(>= button 0)
				   (tw-mouse-button (- 1 state) (+ button 1))]
				[else
				   #f])])
	(tw-mouse-motion x y)

	(unless handled
	  (register-down key button special state x y mod)
	  (input-camera key button special state x y mod width height))))

(define (fluxus-input-release-callback key button special state x y mod)
  (unless (< button 0)
	(tw-mouse-button (- 1 state) (+ button 1)))
  (register-up key button special state x y mod))

(define (fluxus-tw-frame-callback)
  (default-fluxus-frame-callback)
  (for-each
	(lambda (var)
	  (tw-remove-var (car var) (cdr var)))
	variables-to-remove)
  (set! variables-to-remove '())
  (tw-draw))

(override-frame-callback fluxus-tw-frame-callback)

;; StartSectionDoc-en
;; tw - AntTweakBar fluxus module
;;
;; AntTweakBar is a small and easy-to-use C/C++ library that allows programmers
;; to quickly add a light and intuitive graphical user interface into graphic
;; applications to interactively tweak their parameters on-screen.
;;
;; Program variables can be bound to graphical controls that allow users to
;; modify them. Thus, parameters exposed by programmers can be easily modified.
;; They are displayed into the graphical application through one or more
;; embedded windows called tweak bars.
;;
;; The AntTweakBar library mainly targets graphical applications that need a
;; quick way to tune parameters (even in fullscreen mode) and see the result
;; in real-time like 3D demos, games, prototypes, inline editors, debug
;; facilities of weightier graphical applications, etc. 
;;
;; Example:
;; (require "tw.ss")
;; (tw-init)
;; (define bar (tw-new-bar "TweakBar Example"))
;; (define s (box 1.0))
;; (tw-add-var bar "scale" s "min=.1 max=8. step=.05")
;; (every-frame
;;   (begin
;;      (scale (unbox s))
;;      (draw-cube)))
;; EndSectionDoc

(define _twbar-ptr (_cpointer 'twbar))

;; twtype defines
;; does not work as an enum because tw-define-enum adds new types to the list
(define twtype-undef 0)
(define twtype-boolcpp 1)
(define twtype-bool8 2)
(define twtype-bool16 3)
(define twtype-bool32 4)
(define twtype-char 5)
(define twtype-int8 6)
(define twtype-uint8 7)
(define twtype-int16 8)
(define twtype-uint16 9)
(define twtype-int32 10)
(define twtype-uint32 11)
(define twtype-float 12)
(define twtype-double 13)
(define twtype-color32 14)
(define twtype-color3f 15)
(define twtype-color4f 16)
(define twtype-cdstring 17)
(define twtype-stdstring 18)
(define twtype-quat4f 19)
(define twtype-quat4d 20)
(define twtype-dir3f 21)
(define twtype-dir3d 22)

;; StartFunctionDoc-en
;; tw-add-button bar-id name-string [callback-procedure] [def-string]
;; Returns: #t if the button was sucessfully added 
;; Description:
;; This function adds a button entry to a tweak bar. When the button is
;; clicked by a user, the callback function is called.
;; Remark:
;; If you add a button without specifying callback (ie. set to #f),
;; no button icon is displayed. Only the name or label of the button
;; is displayed, and it is not clickable. It looks like a comment. This
;; is a way to add a line of text in a tweak bar. 
;; Example:
;; (tw-add-button bar "hello" (lambda () (display "hello world!")(newline)))
;; (tw-add-button bar "comment1" #f "label='Life is like a box a chocolates'")
;; EndFunctionDoc

(define _tw-button-callback
 	(_fun #:keep #t
		  (client-data : _pointer)
		  -> _void))

(define (tw-add-button bar name [callback #f] [def #f])
  (let ([addbutton (get-ffi-obj "TwAddButton" libtw
						(_fun _twbar-ptr _string _tw-button-callback _pointer _string
							  -> _bool))])
	(addbutton bar name
			   (if callback
				   (lambda (client-data)
						(callback))
				   #f)
			   #f
			   def)))

;; StartFunctionDoc-en
;; tw-add-separator bar-id [name-string] [def-string]
;; Returns: #t if the separator was successfully added to the tweak bar.
;; Description:
;; This function adds a horizontal separator line to a tweak bar. It may
;; be useful if one wants to separate several sets of variables inside a
;; same group.
;; Note that you can also add a line of text in a tweak bar using a special
;; button, see tw-add-button.
;; Example:
;; (tw-add-separator bar)
;; EndFunctionDoc

(define (tw-add-separator bar [name #f] [def #f])
  (let ([addsep (get-ffi-obj "TwAddSeparator" libtw
						(_fun _twbar-ptr _string _string -> _bool))])
	(addsep bar name def)))

;; StartFunctionDoc-en
;; tw-init
;; Returns: #f if an error has occurred
;; Description:
;; This function initializes the AntTweakBar library. It must be called
;; at the beginning of the program.
;; Example:
;; (tw-init)
;; EndFunctionDoc

(define tw-init
  (let ([init (get-ffi-obj "TwInit" libtw
                           (_fun (graphapi : _int)
                                 (device : _pointer)
                                 -> _bool))]
        [inited #f])
    (lambda ()
      (when inited
        (tw-terminate))

      (tw-window-size width height)
      (set! inited (init 1 #f))
	  inited)))

;; tw-window-size width-number height-number
;; Description:
;;  Call this function to inform AntTweakBar of the size of the application
;;  graphics window, or to restore AntTweakBar graphics resources (after a
;;  fullscreen switch for instance).
;; Parameters:
;;   width
;;    Width of the graphics window.
;;   height
;;    Height of the graphics window.
;; Return value:
;;   #f if failed (call tw-get-last-error to retrieve the error).
;;   #t if succeeded.

(define (tw-window-size w h)
  (let ([tw-ws (get-ffi-obj "TwWindowSize" libtw
                     (_fun (width : _int)
                           (height : _int)
                           -> _bool))])
    (tw-ws (inexact->exact (floor w))
           (inexact->exact (floor h)))))

;; tw-get-bar-count
;; Description:
;;  Returns the number of created bars.
;; Return value:
;;  Number of bars.

(define tw-get-bar-count
    (get-ffi-obj "TwGetBarCount" libtw
        (_fun -> _int)))

;; StartFunctionDoc-en
;; tw-new-bar name-string
;; Returns: bar-id
;; Description:
;; Creates a new tweak bar.
;; The AntTweakBar library must have been initialized (by calling tw-init)
;; before creating a bar.
;; Example:
;; (define bar (tw-new-bar "TweakBar"))
;; EndFunctionDoc

(define tw-new-bar
    (get-ffi-obj "TwNewBar" libtw
        (_fun _string
              -> _twbar-ptr)))

;; tw-mouse-button action-int button-int
;; Description:
;;  Call this function to inform AntTweakBar that a mouse button is pressed.
;;  AntTweakBar interprets this event and acts consequently. So tw-mouse-button
;;  has to be called each time your app receives a mouse button event that is
;;  not handled directly by your app.
;; Parameters:
;;  action
;;   Tells if the button is pressed or released. It is one of the TwMouseAction
;;   constants.
;;  button
;;   Tells which button is pressed. It is one of the TwMouseButtonID constants.
;; Return value:
;;  #t if the mouse event has been handled by AntTweakBar,
;;  #f otherwise.

(define tw-mouse-button
    (get-ffi-obj "TwMouseButton" libtw
        (_fun (action : _int)
              (button : _int)
              -> _bool)))

;; tw-mouse-motion mousex-int mousey-int
;; Description:
;;  Call this function to inform AntTweakBar that the mouse has moved.
;;  AntTweakBar interprets this event and acts consequently. So tw-mouse-motion
;;  has to be called each time your app receives a mouse motion event that is
;;  not handled directly by your app.
;; Parameters:
;;  mouse-x
;;   The new X position of the mouse, relative to the left border of the
;;   graphics window.
;;  mouse-y
;;   The new Y position of the mouse, relative to the top border of the
;;   graphics window.
;; Return value:
;;   #t if the mouse event has been handled by AntTweakBar,
;;	 #f otherwise.

(define tw-mouse-motion
    (get-ffi-obj "TwMouseMotion" libtw
        (_fun (mouse-x : _int)
              (mouse-y : _int)
              -> _bool)))

;; tw-mouse-wheel pos-int
;; Description:
;;  Call this function to inform AntTweakBar that the mouse wheel has been used.
;;  AntTweakBar interprets this event and acts consequently. So tw-mouse-wheel
;;  has to be called each time your app receives a mouse wheel event that is
;;  not handled directly by your app.
;; Parameters:
;;  pos
;;   The new position of the wheel.
;; Return value:
;;   #t if the mouse wheel event has been handled by AntTweakBar,
;;   #f otherwise.

(define tw-mouse-wheel
    (get-ffi-obj "TwMouseWheel" libtw
		(_fun (pos : _int)
			  -> _bool)))

;; tw-draw
;; Description:
;;  Draws all the created tweak bars.
;;  This function must be called once per frame, after all the other drawing
;;  calls and just before the application presents (swaps) the frame buffer.
;;  It will draw the bars on top of the other drawings. It tries to backup all
;;  the graphics states that it modifies, and restore them after.
;;  This function is optimized. It aims at having as less impact as possible
;;  on the application frame rate (if the app does other things than to only
;;  display tweak bars, of course).
;; Return value:
;;  #f if failed (call tw-get-last-error to retrieve the error).
;;  #t if succeeded.

(define tw-draw
  (get-ffi-obj "TwDraw" libtw
       (_fun -> _bool)))

;; tw-terminate
;; Description:
;;  Uninitialize the AntTweakBar API. Must be called at the end of the program,
;;  before terminating the graphics API.
;; Return value:
;;  #f if failed (AntTweakBar has not been initialized properly before).
;;  @t if succeeded.

(define tw-terminate
  (get-ffi-obj "TwTerminate" libtw
       (_fun -> _bool)))

;; tw-add-var-cb
;; Description:
;;  This function adds a new variable to a tweak bar by providing CallBack (CB)
;;  functions to access it. If the set-callback parameter is set to #f the
;;  variable is declared Read-Only, so it could not be modified interactively
;;  by the user. Otherwise, it is a Read-Write variable, and could be modified
;;  interactively by the user.
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  type
;;   Type of the variable. It must be one of the TwType constants.
;;  set-callback
;;   The callback function that will be called by AntTweakBar to change the
;;   variable’s value.
;;  get-callback
;;   The callback function that will be called by AntTweakBar to get the
;;   variable’s value.
;;  client-data
;;   Not used in the Scheme module, closure is simpler.
;;  def
;;   An optional definition string used to modify the behavior of this
;;   new entry. This string must follow the variable parameters syntax,
;;   or set to #f to get the default behavior. It could be set or modified
;;   later by calling the tw-define or tw-set-param functions.
;; Return value:
;;  #t if the variable was successfully added to the tweak bar.
;;  #f if an error occurred (call tw-get-last-error to retrieve the error).

(define _tw-set-var-callback
 	(_fun #:keep #t ; prevent callback to be garbage collected
		  (value : _pointer)
		  (client-data : _pointer)
		  -> _void))

(define _tw-get-var-callback
 	(_fun #:keep #t
		  (value : _pointer)
		  (client-data : _pointer)
		  -> _void))

(define tw-add-var-cb
  (get-ffi-obj "TwAddVarCB" libtw
		(_fun (bar : _twbar-ptr)
			  (name : _string)
			  (type : _uint)
			  (set-callback : _tw-set-var-callback)
			  (get-callback : _tw-get-var-callback)
			  (client-data : _pointer)
			  (def : _string)
			  -> _int)))

;; tw-define-enum name-string enum-values-_pointer n-values-_uint
;; Description:
;;  This function creates a new twtype corresponding to a C/C++ enum. Thus it
;;  could be used with tw-add-var* functions to control variables of type enum.
;; Parameters:
;;  name
;;   Specify a name for the enum type (must be unique).
;;  enum-values
;;   An array of structures of type TwEnumVal containing integer values and
;;   their associated labels (pointers to zero terminated strings) corresponding
;;   to the values.
;;  n-values
;;   Number of elements of the enum-values array.
;; Note:
;;  The Scheme module defines enum values through the enum parameter of the def
;;  string of tw-add-var*, while setting the enum-values to #f, and n-values to 0.
;; Return values:
;;   twtype-uint
;;   0 if an error occurred (call tw-get-last-error to retrieve the error).

(define tw-define-enum
  (get-ffi-obj "TwDefineEnum" libtw
		(_fun (name : _string)
			  (enum-values : _pointer)
			  (n-values : _uint)
			  -> _uint)))
 
;; tw-get-last-error
;;  Returns the last error that has occured during a previous AntTweakBar
;;  function call.
;; Return value:
;;  A constant string that describes the error.

(define tw-get-last-error
  (get-ffi-obj "TwGetLastError" libtw
		(_fun -> _string)))

; variables to remove in the format of (bar . name)
; if exception occurs during callbacks
(define variables-to-remove '())

;; tw-remove-var bar-string name-string
;; Description:
;;  This function removes a variable, button or separator from a tweak bar.
;; Parameters:
;;  bar
;;   The tweak bar from which to remove a variable.
;;  name
;;   The name of the variable. It is the same name as the one provided to the
;;   tw-add-var* functions when the variable was added.
;; Return values:
;;  #t if the variable was successfully removed.
;;  #f if an error occurred (call tw-get-last-error to retrieve the error).

(define tw-remove-var
  (get-ffi-obj "TwRemoveVar" libtw
		(_fun (bar : _twbar-ptr)
			  (name : _string)
			  -> _bool)))

;; StartFunctionDoc-en
;; tw-define def-string
;; Returns:
;;  #f if an error has occurred (call tw-get-last-error to retrieve the error).
;;  #t otherwise.
;; Description:
;; This function defines optional parameters for tweak bars and variables.
;; For instance, it allows you to change the color of a tweak bar, to set
;; a min and a max value for a variable, to add an help message that inform
;; users of the meaning of a variable, and so on...
;; Parameters:
;;  def
;;   A string containing one or more parameter assignments (separated by newlines).
;;
;; To define bar parameters, the syntax is:
;;  barName  barParam1=xx barParam2=xx ...
;; where barName is the name of the tweak bar (the same as the one provided to
;; TwNewBar), and barParam n follows the bar parameters syntax.
;;
;; To define variable parameters, the syntax is:
;;  barName/varName  varParam1=xx varParam2=xx ...
;; where barName is the name of the tweak bar (the same as the one provided to
;; tw-new-bar), varName is the name of the variable (the same as the one provided
;; to tw-add-var*), and varParam n follows the variable parameters syntax.
;;
;; If barName or varName contains special characters like spaces or quotation
;; marks, you can surround it by quotes (‘), back-quotes (`) or double-quotes (“).
;;
;; Note:
;; Using a text file to define parameters
;; One or more parameters can be defined by each tw-define call. If you want to
;; assign more than one parameter through one tw-define call, separate them by
;; new lines (\n). This allows, for instance, the read of parameters definition
;; from a file into a string buffer, and then send this string to tw-define.
;;
;; Example:
;; (define bar (tw-new-bar "mybar"))
;; (define wind-vel (box 0))
;; (tw-add-var bar "windvel" wind-vel)
;; (tw-define "mybar/windvel label='Wind velocity'")
;; EndFunctionDoc

(define tw-define
  (get-ffi-obj "TwDefine" libtw
		(_fun (def : _string)
			  -> _bool)))

;; StartFunctionDoc-en
;; tw-add-var bar-id name-string variable-box [def-string]
;; Returns:
;;  #t if the variable was sucessfully added
;;  error raised if a problem occurs
;; Description:
;; This function adds a new variable to a tweak bar by passing the variable in
;; a box. The variable is declared Read-Write (RW), so it could be modified
;; interactively by the user. 
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  var
;;   Boxed variable linked to this entry.
;;  def
;;   An optional definition string used to modify the behavior of this new
;;   entry. This string must follow the variable parameters syntax. It could
;;   be set or modified later by calling the tw-define or tw-set-param
;;   functions.
;; Example:
;; (define zoom (box 1.0))
;; (tw-add-var bar "zoom" zoom)
;; EndFunctionDoc

(define (tw-add-var bar name bvar [def ""])
  (let* ([type->twtype (hash _float twtype-float
							 _int twtype-int32
							 _bool twtype-boolcpp)]
		 [type->conv (hash _float exact->inexact	; conversions to c-types
						   _int (compose inexact->exact round)
						   _bool (lambda (x) x))]
		 [var (unbox bvar)]
		 [type (cond [(and (number? var)
						   (inexact? var))
					  	_float]
					 [(and (number? var)
						   (exact? var))
					  	_int]
					 [(boolean? var)
					  	_bool]
					 [else
					   #f])]
		 [conv (hash-ref type->conv (quasiquote (unquote type)) #f)])
	(if type
	  	(begin
		  	(hash-set! tw-vars name bvar)
			(tw-add-var-cb bar name (hash-ref type->twtype (quasiquote (unquote type)))
						   (lambda (value-ptr client-data)
							 (set-box! bvar (ptr-ref value-ptr type)))
						   (lambda (value-ptr client-data)
							 (with-handlers ([exn:fail?
											   (lambda (exn)
												 (printf "~a~nVariable removed.~n" exn)
												 (set! variables-to-remove (cons (cons bar name) variables-to-remove)))])
								 (ptr-set! value-ptr type (conv (unbox bvar)))))
						   #f
						   def))
		(error 'tw-add-var "unknow type variable ~a" var))))

;; tw-add-vector
;; helper function to add dir3f, color3f and quat4f types

(define (tw-add-vector bar name twtype size bvar [def ""])
	(hash-set! tw-vars name bvar)
	(tw-add-var-cb bar name twtype 
				   (lambda (value-ptr client-data)
					 (set-box! bvar (list->vector
									  (for/list ([i (in-range size)])
										  (ptr-ref value-ptr _float i)))))
				   (lambda (value-ptr client-data)
					 (with-handlers ([exn:fail?
									   (lambda (exn)
										 (printf "~a~nVariable removed.~n" exn)
										 (set! variables-to-remove (cons (cons bar name) variables-to-remove)))])
						 (let ([v (unbox bvar)])
						   	(for ([i (in-range size)])
								 (ptr-set! value-ptr _float i (exact->inexact (vector-ref v i)))))))
				   #f
				   def))

;; StartFunctionDoc-en
;; tw-add-dir bar-id name-string variable-vector-box [def-string]
;; Returns:
;;  #t if the variable was sucessfully added
;;  error raised if a problem occurs
;; Description:
;; This function adds a new direction variable represented by
;; a 3 element vector.
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  var
;;   Boxed 3 element vector variable linked to this entry.
;;  def
;;   An optional definition string used to modify the behavior of this new
;;   entry. This string must follow the variable parameters syntax. It could
;;   be set or modified later by calling the tw-define or tw-set-param
;;   functions.
;; Example:
;; (define dir (box (vector 1 0 0))
;; (tw-add-dir bar "direction" dir)
;; EndFunctionDoc

(define (tw-add-dir bar name bvar [def ""])
  (let ([var (unbox bvar)])
	(if (and (vector? var)
			 (= (vector-length var) 3))
	  	(tw-add-vector bar name twtype-dir3f 3 bvar def)
		(error 'tw-add-dir "3 element vector required, got ~a" var))))

;; StartFunctionDoc-en
;; tw-add-colour bar-id name-string colour-box [def-string]
;; Returns:
;;  #t if the variable was sucessfully added
;;  error raised if a problem occurs
;; Description:
;; This function adds a new colour variable represented by
;; a 3 or 4 element vector.
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  var
;;   Boxed 3 or 4 element vector variable linked to this entry.
;;  def
;;   An optional definition string used to modify the behavior of this new
;;   entry. This string must follow the variable parameters syntax. It could
;;   be set or modified later by calling the tw-define or tw-set-param
;;   functions.
;; Example:
;; (define col (box (vector 1 0 0))
;; (tw-add-colour bar "Colour" col)
;; EndFunctionDoc

(define (tw-add-colour bar name bvar [def ""])
  (let ([var (unbox bvar)])
	(if (and (vector? var)
			 (<= 3 (vector-length var) 4))
	  	(tw-add-vector bar name (if (= (vector-length var) 3)
								  twtype-color3f
								  twtype-color4f)
					   (vector-length var) bvar def)
		(error 'tw-add-colour "3 or 4 element vector required, got ~a" var))))

;; StartFunctionDoc-en
;; tw-add-quat bar-id name-string quaternion-box [def-string]
;; Returns:
;;  #t if the variable was sucessfully added
;;  error raised if a problem occurs
;; Description:
;; This function adds a new quaternion variable represented by
;; a 4 element vector.
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  var
;;   Boxed 4 element vector variable linked to this entry.
;;  def
;;   An optional definition string used to modify the behavior of this new
;;   entry. This string must follow the variable parameters syntax. It could
;;   be set or modified later by calling the tw-define or tw-set-param
;;   functions.
;; Example:
;; (define rotation (box (vector 0 0 0 1)))
;; (tw-add-quat bar "Rotation" rotation)
;; (every-frame
;; 		(with-state
;; 			(concat (qtomatrix rotation))
;; 			(draw-cube)))
;; EndFunctionDoc

(define (tw-add-quat bar name bvar [def ""])
  (let ([var (unbox bvar)])
	(if (and (vector? var)
			 (= (vector-length var) 4))
	  	(tw-add-vector bar name twtype-quat4f
					   4 bvar def)
		(error 'tw-add-quat "4 element quaternion vector required, got ~a" var))))

;; StartFunctionDoc-en
;; tw-add-list bar-id name-string variable-box choices-list-of-symbols [def-string]
;; Returns:
;;  #t if the variable was sucessfully added
;;  error raised if a problem occurs
;; Description:
;; This function adds an option list variable. The option choices are represented
;; by a list of symbols.
;; Parameters:
;;  bar
;;   The tweak bar to which adding a new variable.
;;  name
;;   The name of the variable. It will be displayed in the tweak bar if no
;;   label is specified for this variable. It will also be used to refer to
;;   this variable in other functions, so choose a unique, simple and short
;;   name and avoid special characters like spaces or punctuation marks.
;;  variable
;;   Boxed symbol variable linked to this entry.
;;  choices
;;   List of choices as symbols.
;;  def
;;   An optional definition string used to modify the behavior of this new
;;   entry. This string must follow the variable parameters syntax. It could
;;   be set or modified later by calling the tw-define or tw-set-param
;;   functions.
;; Example:
;; (define season (box 'summer))
;; (tw-add-list bar "Season" season '(summer fall winter spring))
;; EndFunctionDoc

(define (tw-add-list bar name bvar choices [def ""])
  (let* ([def (foldl
				(lambda (i result)
				  (string-append result " "
								 (number->string i) " {"
								 (symbol->string (list-ref choices i)) "}"
								 (if (< i (sub1 (length choices))) "," "'")))
				(string-append def " enum='")
				(build-list (length choices) values))]
		 [enum-assoc (map (lambda (s i)
							(cons s i))
						  choices
						  (build-list (length choices) values))]
		 [enum-uint (tw-define-enum (string-append name "-type") #f 0)])
	(hash-set! tw-vars name bvar)
	(tw-add-var-cb bar name enum-uint
				   (lambda (value-ptr client-data)
					 (set-box! bvar (list-ref choices (ptr-ref value-ptr _uint))))
				   (lambda (value-ptr client-data)
					 (with-handlers ([exn:fail?
									   (lambda (exn)
										 (printf "~a~nVariable removed.~n" exn)
										 (set! variables-to-remove (cons (cons bar name) variables-to-remove)))])
						 (ptr-set! value-ptr _uint (cdr (assoc (unbox bvar) enum-assoc)))))
				   #f
				   def)))

;; save/load variable plist
(require xml/plist)

; variable hash table name->boxed var, TODO: add bar name
(define tw-vars (make-hash))

;; StartFunctionDoc-en
;; tw-save-config [filename]
;; Returns:
;;  void
;; Description:
;; Saves all variables to an xml/plist config file.
;; Example:
;; (define zoom (box 1.0))
;; (tw-add-var bar "zoom" zoom)
;; (tw-save-config)
;; EndFunctionDoc

(define (tw-save-config [filename "config.plist"])
  (let ([hash-dict (cons 'dict
						(hash-map
						  tw-vars
						  (lambda (k bvar)
							(let* ([var (unbox bvar)])
								(list 'assoc-pair k
									  (cond [(and (number? var)
												  (inexact? var)) (list 'real var)]
											[(and (number? var)
												  (exact? var)) (list 'integer var)]
											[(symbol? var) (symbol->string var)]
											[(boolean? var) (if var
															  (list 'true)
															  (list 'false))]
											[(vector? var) (cons 'array
																 (map
																   (lambda (e)
																	 (list 'real e))
																   (vector->list var)))]))))))]
		[port (open-output-file filename #:exists 'replace)])
	(write-plist hash-dict port)
	(close-output-port port)))

;; StartFunctionDoc-en
;; tw-load-config [filename]
;; Returns:
;;  void
;; Description:
;; Loads variable config plist file saved by tw-save-config.
;; Example:
;; (define zoom (box 1.0))
;; (tw-add-var bar "zoom" zoom)
;; (tw-load-config)
;; EndFunctionDoc

(define (tw-load-config [filename "config.plist"])
  (letrec ([port (with-handlers ([exn:fail:filesystem?
								(lambda (e)
								  (printf "tw-load-config: ~a~n" (exn-message e))
								  #f)])
					(open-input-file filename))]
		   [plist (when port
					  (read-plist port))]
		   [read-pl-value (lambda (pl)
								(if (string? pl)
								  (string->symbol pl)
								  (case (car pl)
									[(true) #t]
									[(false) #f]
									[(integer real) (cadr pl)]
									[(array) (list->vector (map read-pl-value (cdr pl)))])))]
		   [read-assoc-pair (lambda (ap)
								(let ([name (cadr ap)]
									  [val (caddr ap)])
								  (cons name (read-pl-value val))))]
		   [read-dict (lambda (plist) ; convert plist to (name value) pairs
						  (cond [(empty? plist) '()]
								[(eq? (car plist) 'dict)
								 	(read-dict (cdr plist))]
								[else
								  (cons (read-assoc-pair (car plist))
										(read-dict (cdr plist)))]))])
	(when port
		(close-input-port port)
		;(displayln plist)
		(for-each
		  (lambda (kvp)
			(let* ([key (car kvp)]
				   [value (cdr kvp)]
				   [bvar (hash-ref tw-vars key #f)])
			  (if bvar
				(set-box! bvar value)
				(printf "tw-load-config: variable ~a not found.~n" key))))
		  (read-dict plist)))))

