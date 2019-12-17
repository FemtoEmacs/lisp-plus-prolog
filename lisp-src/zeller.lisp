(defun winter(m) (floor (- 14 m) 12))

(defun roman-order(m)
   (+ m (* (winter m) 12) -2))

(defun zr(y m day)
  (let* ( (roman-month (roman-order m))
          (roman-year (- y (winter m)))
          (century (floor roman-year 100))
          (decade (mod roman-year 100)) )
     (mod (+ day decade
             (floor (- (* 13 roman-month) 1) 5)
             (floor decade 4)
             (floor century 4)
             (* century -2)) 7)) )
