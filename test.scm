(use gauche.test)
(use gauche.process)
(use gauche.threads)
(use file.util)
(use srfi-1)

(test-start "myc compiler tests")

;; コンパイルと実行のヘルパー関数
(define (run-test expected input id)
  (let* ((tmp-s (format "tmp-~a.s" id))
         (tmp-exe (format "tmp-~a" id))
         (test-helper "build/test_helper.o")
         (myc "./build/myc"))
    
    (unwind-protect
     (begin
       ;; 1. Compile C to assembly
       (let ((p (run-process `(,myc ,input) :output tmp-s :wait #t)))
         (unless (zero? (process-exit-status p))
           (error "Compile failed" input)))
       
       ;; 2. Assemble and link with test_helper.o
       (let ((p (run-process `(cc -o ,tmp-exe ,tmp-s ,test-helper) :wait #t)))
         (unless (zero? (process-exit-status p))
           (error "Link failed" input)))
       
       ;; 3. Run the compiled executable and get exit status
       (let ((p (run-process (list (string-append "./" tmp-exe)) :wait #t)))
         (sys-wait-exit-status (process-exit-status p))))
     
     ;; Cleanup
     (begin
       (when (file-exists? tmp-s) (sys-unlink tmp-s))
       (when (file-exists? tmp-exe) (sys-unlink tmp-exe))))))

;; テストケースのリスト (expected . input)
(define test-cases
  '(
    (0 . "main() { return 0; }")
    (123 . "main() { return 123; }")
    (6 . "main() { return 1+2+3; }")
    (2 . "main() { return 1-2+3; }")
    (24 . "main() { return (4*2)*3; }")
    (20 . "main() { return (30*2)/3; }")
    (5 . "main() { return -3+(4*2); }")
    (11 . "main() { return +3+(4*2); }")
    (1 . "main() { return 2==2; }")
    (0 . "main() { return 2!=2; }")
    (0 . "main() { return 2<2; }")
    (0 . "main() { return 3<2; }")
    (1 . "main() { return 2<=20; }")
    (0 . "main() { return 2>2; }")
    (1 . "main() { return 3>2; }")
    (7 . "main() { 1-2+3; return 7; }")
    (3 . "main() { a = 3; return a; }")
    (22 . "main() { b = 5*6-8; return b; }")
    (14 . "main() { a = 3; b = 5*6-8; b = a+b/2; return b; }")
    (6 . "main() { foo=1; bar=5; return foo+bar; }")
    (3 . "main() { foo=3; bar=5; return foo; }")
    (10 . "main() { foo=0; if(foo==0) foo=10; return foo; }")
    (10 . "main() { foo=0; if(foo==0) { foo=10; } return foo; }")
    (10 . "main() { foo=0; if(foo==0) foo=10; else foo=5; return foo; }")
    (10 . "main() { foo=1; if(foo) foo=10; else foo=5; return foo; }")
    (5 . "main() { foo=1; if(foo==0) foo=10; else foo=5; return foo; }")
    (5 . "main() { foo=0; while(foo<5) foo=foo+1; return foo; }")
    (9 . "main() { foo=0; for(bar=0; bar<10; bar=bar+1) foo=bar; return foo; }")
    (10 . "main() { foo=0; for(bar=0; bar<10; bar=bar+1) foo=bar; return bar; }")
    (42 . "main() { return foo(); }")
    (10 . "main() { return add(3, 7); }")
    (21 . "main() { return add6(1, 2, 3, 4, 5, 6); }")
    (7 . "main() { return add2(3, 4); } add2(x, y) { return x + y; }")
    (1 . "main() { return sub2(4, 3); } sub2(x, y) { return x - y; }")
    (55 . "main() { return fib(10); } fib(n) { if(n<=1) return n; return fib(n-1) + fib(n-2); }")
  ))

;; 並列実行の実行
(define num-cpus 10)

(test-section "Parallel automated testing")

;; 結果を収集するためのスレッド関数 (カスタムキューを使用した並行実行)
(define (run-parallel-tests)
  (let* ((len (length test-cases))
         (results (make-vector len #f))
         ;; (index . (expected . input)) のリストを作成
         (jobs (let loop ((i 0) (lst test-cases) (res '()))
                 (if (null? lst)
                     (reverse res)
                     (loop (+ i 1) (cdr lst) (cons (cons i (car lst)) res)))))
         (jobs-mutex (make-mutex))
         (threads '()))
    
    (define (worker)
      (let loop ()
        (let ((job #f))
          (mutex-lock! jobs-mutex)
          (unless (null? jobs)
            (set! job (car jobs))
            (set! jobs (cdr jobs)))
          (mutex-unlock! jobs-mutex)
          
          (when job
            (let* ((i (car job))
                   (case (cdr job))
                   (expected (car case))
                   (input (cdr case)))
              (let ((actual (guard (e [else (print "Error in test " i ": " (~ e'message)) #f])
                              (run-test expected input i))))
                (vector-set! results i actual)))
            (loop)))))

    ;; ワーカーを指定された数だけ起動 (iota がない場合に備えて手動でリスト作成)
    (let loop-w ((n num-cpus))
      (if (> n 0)
          (begin
            (set! threads (cons (thread-start! (make-thread worker)) threads))
            (loop-w (- n 1)))))
    
    ;; 全てのワーカーの終了を待機
    (for-each thread-join! threads)
    results))

(let ((results (run-parallel-tests)))
  (let loop-r ((i 0) (lst test-cases))
    (unless (null? lst)
      (test* (cdar lst) (caar lst) (vector-ref results i))
      (loop-r (+ i 1) (cdr lst)))))

(test-end)
