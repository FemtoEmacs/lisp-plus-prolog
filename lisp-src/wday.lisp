(defun zeller(y m day)
  (let* ( (roman-month (if (< m 3) (+ m 10) (- m 2)))
          (roman-year (if (< m 3) (- y 1) y))
          (century (floor roman-year 100))
          (decade (mod roman-year 100)) )
    (mod (+ day decade
           (floor (- (* 13 roman-month) 1) 5)
           (floor decade 4)
           (floor century 4)
           (* century -2)) 7)) )
