(defun on-enter ()
  (send `(level goal ,(car args) hide)))
(defun on-leave ()
  ;; TODO(#615): showing goals is not implemented
  (send `(level goal ,(car args) show)))

