mkdir -p coverage && gcovr --root ./.. --filter "\.\./src/.*" --html --html-details -o coverage/coverage.html
echo -e "Find coverage report at: \"$(realpath ./)/coverage/coverage.html\""