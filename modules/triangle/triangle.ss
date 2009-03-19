;; Copyright (C) 2009 Gabor Papp
;;
;; Interface to Jonathan Richard Shewchuk's Triangle library
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

(provide triangulate)

(define libtriangle (ffi-lib "libtriangle"))

(define _real _double)

;;; FIXME: tagged pointers
(define-cstruct _triangulateio (
	[pointlist _pointer]				; REAL *pointlist;					/* In / out */
	[pointattributelist _pointer]		; REAL *pointattributelist;			/* In / out */
	[pointmarkerlist _pointer]			; int *pointmarkerlist;				/* In / out */
	[numberofpoints _int]				; int numberofpoints;				/* In / out */
	[numberofpointattributes _int]		; int numberofpointattributes;		/* In / out */

	[trianglelist _pointer]				; int *trianglelist;				/* In / out */
	[triangleattributelist _pointer]	; REAL *triangleattributelist;		/* In / out */
	[trianglearealist _pointer]			; REAL *trianglearealist;			/* In only */
	[neighborlist _pointer]				; int *neighborlist;				/* Out only */
	[numberoftriangles _int]			; int numberoftriangles;			/* In / out */
	[numberofcorners _int]				; int numberofcorners;				/* In / out */
	[numberoftriangleattributes _int]	; int numberoftriangleattributes;	/* In / out */

	[segmentlist _pointer]				; int *segmentlist;					/* In / out */
	[segmentmarkerlist _pointer]		; int *segmentmarkerlist;			/* In / out */
	[numberofsegments _int]				; int numberofsegments;				/* In / out */

	[holelist _pointer]					; REAL *holelist;		/* In / pointer to array copied out */
	[numberofholes _int]				; int numberofholes;                     /* In / copied out */

	[regionlist _pointer]				; REAL *regionlist;		/* In / pointer to array copied out */
	[numberofregions _int]				; int numberofregions;					 /* In / copied out */
	
	[edgelist _pointer]					; int *edgelist;								/* Out only */
	[edgemarkerlist _pointer]			; int *edgemarkerlist;	 /* Not used with Voronoi diagram; out only */
	[normlist _pointer]					; REAL *normlist;		/* Used only with Voronoi diagram; out only */
	[numberofedges _int]				; int numberofedges;							/* Out only */
	))

(define (make-triangulateio-default)
  (make-triangulateio #f #f #f 0 0
					  #f #f #f #f 0 0 0
					  #f #f 0
					  #f 0
					  #f 0
					  #f #f #f 0))

;; triangle-report io flags
;; io - triangulateio struct
;; flags - a list of symbols containing 'markers, 'triangles,
;; 			'neighbors, 'segments, 'edges, 'norms
(define (triangulate-report io flags)
  (define marker (member 'markers flags))
  (define triangles (member 'triangles flags))
  (define neighbors (member 'neighbors flags))
  (define segments (member 'segments flags))
  (define edges (member 'edges flags))
  (define norms (member 'norms flags))

  (let* ([np (triangulateio-numberofpoints io)]
		 [p (cvector->list (make-cvector* (triangulateio-pointlist io) _real (* np 2)))]
		 [npa (triangulateio-numberofpointattributes io)]
		 [pa (cvector->list (make-cvector* (triangulateio-pointattributelist io) _real (* np npa)))]
		 [pm (if marker
			 	(cvector->list (make-cvector* (triangulateio-pointmarkerlist io) _int np))
				#f)])
		(for ([i (in-range np)])
			  (printf "point ~a: ~a, ~a" i
					  					(list-ref p (* i 2))
					  					(list-ref p (+ (* i 2) 1)))
			  (when (> npa 0)
				(printf " attributes:"))
			  (for ([j (in-range npa)])
				  	(printf " ~a" (list-ref pa (+ (* i npa) j))))
			  (when marker
					(printf "  marker: ~a" (list-ref pm i)))
			  (newline))
		(newline))

  (when (or triangles neighbors)
	(let* ([nt (triangulateio-numberoftriangles io)]
		   [nc (triangulateio-numberofcorners io)]
		   [t  (if (> nt 0)
				 (cvector->list (make-cvector* (triangulateio-trianglelist io) _int (* nt nc)))
				 #f)]
		   [nta (triangulateio-numberoftriangleattributes io)]
		   [ta (if (> nta 0)
				 (cvector->list (make-cvector* (triangulateio-triangleattributelist io) _real (* nta nt)))
				 #f)]
		   [tn (if (> nt 0)
				 (cvector->list (make-cvector* (triangulateio-neighborlist io) _int (* 3 nt)))
				 #f)])
	  (for ([i (in-range nt)])
		  (when triangles
			   (printf "triangle ~a points:" i)
			   (for ([j (in-range nc)])
					(printf " ~a" (list-ref t (+ (* i nc) j))))
			   (when (> nta 0)
					(printf " attributes"))
			   (for ([j (in-range nta)])
				  	(printf " ~a" (list-ref ta (+ (* i nta) j))))
			   (newline))
		  (when neighbors
				(printf "triangle ~a neighbors:" i)
				(for ([j (in-range 3)])
					(printf " ~a" (list-ref tn (+ (* i 3) j))))
					(newline)))
	  (newline)))

  (when segments
	  (let* ([ns (triangulateio-numberofsegments io)]
			 [s (cvector->list (make-cvector* (triangulateio-segmentlist io) _int (* ns 2)))]
			 [sm (if marker
					(cvector->list (make-cvector* (triangulateio-segmentmarkerlist io) _int ns))
					#f)])
			(for ([i (in-range ns)])
				  (printf "segment ~a points: ~a, ~a" i
											(list-ref s (* i 2))
											(list-ref s (+ (* i 2) 1)))
				  (when marker
						(printf "  marker: ~a" (list-ref sm i)))
				  (newline))
			(newline)))

  (when edges
	  (let* ([ne (triangulateio-numberofedges io)]
			 [e (cvector->list (make-cvector* (triangulateio-edgelist io) _int (* ne 2)))]
			 [em (if marker
					(cvector->list (make-cvector* (triangulateio-edgemarkerlist io) _int ne))
					#f)]
			 [en (if (and norms (triangulateio-normlist io))
					(cvector->list (make-cvector* (triangulateio-normlist io) _real (* ne 2)))
					#f)])
			(for ([i (in-range ne)])
				  (printf "edge ~a points: ~a, ~a" i
											(list-ref e (* i 2))
											(list-ref e (+ (* i 2) 1)))
				  (when (and norms (= (list-ref e (+ (* i 2) 1)) -1))
						(printf " norms: ~a ~a" (list-ref en (* i 2))
										 (list-ref en (+ (* i 2) 1))))
				  (when marker
						(printf "  marker: ~a" (list-ref em i)))
				  (newline))
			(newline)))
  )

(define triangulate-helper
    (get-ffi-obj "triangulate" libtriangle
        (_fun (switches : _string)
            (in : _triangulateio-pointer)
            (out : _triangulateio-pointer)
            (vorout : _triangulateio-pointer)
			-> _void)))


;; free all non-NULL pointers of all objects in list
;; l - list of triangulateio objects
(define (free-triangulateio-pointers l)
	  (for* ([get-pointer (list triangulateio-pointlist
								triangulateio-pointattributelist
								triangulateio-pointmarkerlist
								triangulateio-trianglelist
								triangulateio-triangleattributelist
								triangulateio-trianglearealist
								triangulateio-neighborlist
								triangulateio-segmentlist
								triangulateio-segmentmarkerlist
								triangulateio-holelist
								triangulateio-regionlist
								triangulateio-edgelist
								triangulateio-edgemarkerlist
								triangulateio-normlist)]
			 [io l])

			(let ([p (get-pointer io)])
			  (when p
				(free p)))))


(define (triangulate-test)
	(define numberofpoints 4)
	(define pointlist (list 0.0 0.0 1.0 0.0 1.0 10.0 0.0 10.0))
	(define numberofpointattributes 1)
	(define pointattributelist (list 0.0 1.0 11.0 10.0))
	(define pointmarkerlist (list 0 2 0 0))
	(define numberofsegments 0)
	(define numberofholes 0)
	(define numberofregions 1)
	(define regionlist (list 0.5 5.0 7.0 0.1))

	(let ([in (make-triangulateio-default)]
		  [out (make-triangulateio-default)]
		  [voro (make-triangulateio-default)])
		(set-triangulateio-numberofpoints! in numberofpoints)
		(set-triangulateio-pointlist! in (cvector-ptr (list->cvector pointlist _real)))
		(set-triangulateio-pointmarkerlist! in (cvector-ptr (list->cvector pointmarkerlist _int)))
		(set-triangulateio-numberofpointattributes! in numberofpointattributes)
		(set-triangulateio-pointattributelist! in (cvector-ptr (list->cvector pointattributelist _real)))
		(set-triangulateio-numberofregions! in numberofregions)
		(set-triangulateio-regionlist! in (cvector-ptr (list->cvector regionlist _real)))
	
		(triangulate-report in '(markers triangles))
		(triangulate-helper "pczAevn" in out voro)
		(printf "initial triangulation~n~n")
		(triangulate-report out '(markers triangles neighbors segments edges))
		(printf "initial voronoi diagram~n~n")
		(triangulate-report voro '(edges norms))

		(free-triangulateio-pointers (list out voro))))


;; calculates delaunay triangulation/voronoi diagram
;;
;; points - list of 2d vectors
;; flags  - list of symbols specifying the needed output, accepted symbols are
;; 			'convex-hull, 'delaunay-triangles, 'delaunay-edges,
;; 			'voronoi-points, 'voronoi-edges
;; returns - values of output types (points, edges, triangles)
;; 				- convex hull - list of point indices
;; 				- list of points - list of 2d vectors
;; 				- list of edges - list of point pair indices of each edge,
;; 		     		if the second index is a pair? it means a special edge that
;; 		     		is an infinite ray with only one endpoint, the pair
;; 		     		indicates the direction of the infinite ray
;; 				- list of triangles - list of list of 3 point indices
;; 				- list of cells - list of list of point indices in clockwise order
;; 								  or #f for infinite cells
(define (triangulate points flags)
  (let ([in (make-triangulateio-default)]
		[out (make-triangulateio-default)]
		[voro (make-triangulateio-default)]
		[pointlist (foldl (lambda (v l) (append l (vector->list v))) '() points)])
	(set-triangulateio-numberofpoints! in (length points))
	(set-triangulateio-pointlist! in (cvector-ptr (list->cvector pointlist _real)))

	(when (> (length points) 2)	; triangle exists if called with less than 3 points
		(triangulate-helper "Qpzcevn" in out voro))
	
	(let* ([ont (triangulateio-numberoftriangles out)]
		   [onc (triangulateio-numberofcorners out)]
		   [ot  (if (> ont 0)
					(cvector->list (make-cvector* (triangulateio-trianglelist out) _int (* ont onc)))
					#f)]
		   [one (triangulateio-numberofedges out)]
		   [oe (cvector->list (make-cvector* (triangulateio-edgelist out) _int (* one 2)))]
		   [onp (triangulateio-numberofpoints out)]
		   [om (cvector->list (make-cvector* (triangulateio-pointmarkerlist out) _int onp))]

		   [vnp (triangulateio-numberofpoints voro)]
		   [vp (cvector->list (make-cvector* (triangulateio-pointlist voro) _real (* vnp 2)))]
		   [vne (triangulateio-numberofedges voro)]
		   [ve (cvector->list (make-cvector* (triangulateio-edgelist voro) _int (* vne 2)))]
		   [ven (cvector->list (make-cvector* (triangulateio-normlist voro) _real (* vne 2)))]
		   [neighbors (cvector->list (make-cvector* (triangulateio-neighborlist out) _int (* vnp 3)))]

		   [output-list
				(for/list ([f flags])
					(case f
					  ['convex-hull			(for/list ([i (in-range onp)]
													   #:when (= (list-ref om i) 1)) 
													  i)]

					  ['delaunay-triangles	(for/list ([i (in-range ont)])
												(for/list ([j (in-range onc)])
													(list-ref ot (+ (* i onc) j))))]

					  ['delaunay-edges		(for/list ([i (in-range one)])
												(cons (list-ref oe (* i 2))
													  (list-ref oe (+ (* i 2) 1))))]
					  
					  ['voronoi-points		(for/list ([i (in-range vnp)])
												(vector (list-ref vp (* i 2))
														(list-ref vp (+ (* i 2) 1))))]

					  ['voronoi-edges		(for/list ([i (in-range vne)])
												(let ([a (list-ref ve (* i 2))]
													  [b (list-ref ve (+ (* i 2) 1))])
													(cons a (if (= b -1) ; store direction if it is an infinite ray
															 (cons (list-ref ven (* i 2))
																   (list-ref ven (+ (* i 2) 1)))
															  b))))]
					  ['voronoi-cells
						  (for/list ([i (in-range (length points))])
								(let* ([dedge	; search for a delaunay edge that has input vertex i as an endpoint
												(do ([j 0 (add1 j)]
													 [found #f])
												   ((or (>= j one) found) found)

												   (when (or (= (list-ref oe (* j 2)) i)
															 (= (list-ref oe (+ (* j 2) 1)) i))
													 (set! found j)))]

									   [vstartp	; one endpoint of the delaunay edge's dual voronoi edge
										 		(if dedge
													(list-ref ve (* dedge 2))
													#f)] ; no delaunay edge found - too few points

									   [poly	; step through the cell vertices, starting from vedgep, following the
												; neighbors of the vertices until we get back to the starting point or
												; we reach -1 that denotes an infinite cell
											(if vstartp
												(do ([vp vstartp]
													 [poly '()])
													((or (= vp -1) ; infinite cell
														 (and (= vp vstartp) (not (empty? poly)))) ; got back to the start
														(if (= vp -1)
														  #f	; return #f for infinite cells
														  poly))
													; add voronoi point to cell, since it is put into the front the
													; winding order is reversed, the output should be counter-clockwise,
													; so we traverse the edges in clockwise order
													(set! poly (cons vp poly)) 

													; find the neighbor of the voronoi point to the right
													(set! vp (list-ref neighbors (+ (* 3 vp)
																; find p in the triangle of the voronoi point, vp
																; the previous index is the index in the neighbor list
																; containing the neighbor to the right (clockwise order),
																; because the 1st neighbor is opposite the first corner
																; of the triangle, and so on
																(let ([tri0 (list-ref ot (* vp 3))]
																	  [tri1 (list-ref ot (+ (* vp 3) 1))]
																	  [tri2 (list-ref ot (+ (* vp 3) 2))])
																  (cond [(= tri0 i) 2]
																		[(= tri1 i) 0]
																		[(= tri2 i) 1]
																		[else (error "cannot find point in triangle")]))))))
												#f)]) ; no polygon - too few points

									   poly))]

						  [else (error "triangulate expects one of 'convex-hull, 'delaunay-triangles, 'delaunay-edges, 'voronoi-points, 'voronoi-edges, 'voronoi-cells, but got " f)]))])

		  (free-triangulateio-pointers (list out voro))

		  ; return values
		  (apply values output-list))))

