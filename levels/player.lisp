(set jump-count 0)

(defun on-jump ()
  (set jump-count (+ jump-count 1))
  (when (> jump-count 3)
    ;; TODO(#553): label is hidden without any animation
    (hide-label "label_space_to_jump")))
