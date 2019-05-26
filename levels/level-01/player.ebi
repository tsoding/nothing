(set jump-count 0)

(defun on-jump ()
  (set jump-count (+ jump-count 1))
  (when (> jump-count 3)
    (send `(game level label "label_space_to_jump" hide))))
