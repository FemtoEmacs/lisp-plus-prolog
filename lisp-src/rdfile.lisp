;; File: (load "rdfile.lisp")

(defmacro nlet(n letargs &rest body)
  `(labels ((,n ,(mapcar #'car letargs) ,@body))
      (,n ,@(mapcar #'cadr letargs)) ))

(defun iscomment(s)
  (and (> (length s) 0)
       (char= (aref s 0) #\;)))

(defun rdLines(fname)
  (with-open-file (stream fname)
    (nlet next-line 
       ( (line (read-line stream nil)) )
       (if (null line) nil
	 (cons line
	    (next-line (read-line stream nil))) )) ))

(defun getsemi(fname)
  (remove-if-not #'iscomment (rdLines fname)))

