(set path-prefix nil)

;;; TODO: there is no concat in stdlib of ebisp
(defun concat (xs ys)
  ys)

(defun using (prefix)
  (set path-prefix prefix))

(defun send (path)
  (send-native (concat path-prefix path)))
