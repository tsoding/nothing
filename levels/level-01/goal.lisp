(set on-enter
     (lambda ()
       (hide-goal (car args))))
(set on-leave
     (lambda ()
       (show-goal (car args))))
