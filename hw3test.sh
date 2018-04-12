$1 p1a.txt -l -a >stu-p1a-la.txt
diff stu-p1a-la.txt base-p1a-la.txt
echo "Case #1a >p1a.txt -l -a Complete"

$1 p1a.txt -l -a -v >stu-p1a-lav.txt
diff stu-p1a-lav.txt base-p1a-lav.txt
echo "Case #1b >p1a.txt -l -a -v Complete"

$1 p2a.txt -l -a >stu-p2a-la.txt
diff stu-p2a-la.txt base-p2a-la.txt
echo "Case #2a >p2a.txt -l -a Complete"

$1 p2a.txt -l -a -v >stu-p2a-lav.txt
diff stu-p2a-lav.txt base-p2a-lav.txt
echo "Case #2b >p2a.txt -l -a -v Complete"

$1 p3a.txt -l -a >stu-p3a-la.txt
diff stu-p3a-la.txt base-p3a-la.txt
echo "Case #3a >p3a.txt -l -a Complete"

$1 p3a.txt -l -a -v >stu-p3a-lav.txt
diff stu-p3a-lav.txt base-p3a-lav.txt
echo "Case #3b >p3a.txt -l -a -v Complete"

$1 p4a.txt -l -a >stu-p4a-la.txt
diff stu-p4a-la.txt base-p4a-la.txt
echo "Case #4a >p4a.txt -l -a Complete"

$1 p4a.txt -l -a -v >stu-p4a-lav.txt
diff stu-p4a-lav.txt base-p4a-lav.txt
echo "Case #4b >p4a.txt -l -a -v Complete"
