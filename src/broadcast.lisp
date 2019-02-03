(set path-prefix nil)

(defun using (prefix)
  (set path-prefix prefix))

(defun send (path)
  (send-native (append path-prefix path)))
