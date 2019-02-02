(defvar path-prefix '())

(defun using (prefix)
  (set path-prefix prefix))

(defun send (path)
  (send-native (concat path-prefix path)))
