(defun on-enter ()
  (send `(level goal ,(car args) hide)))
(defun on-leave ()
  ;; TODO: showing goals is not implemented
  (send `(level goal ,(car args) show)))

