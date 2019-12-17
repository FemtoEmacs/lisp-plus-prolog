(defun vowelp(x)
   (cond ( (or (char= x #\a)(char= x #\e)(char= x #\i) 
           (char= x #\o)(char= x #\u)) T)
         (T nil)))
