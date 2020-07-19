default: format

.env:
	grep -Eo 'sysenv.[A-Z_]*' platformio.ini | sed -E 's/sysenv.(.*)/\1=/' >.env
	@echo "/!\ Sample environment created, please adjust!"
	false

format:
	clang-format -i --style Google -Werror src/*

deploy: .env
	envrun -- platformio run -t upload

monitor:
	platformio device monitor
