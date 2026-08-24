.PHONY: install setup-cert dev-backend dev-frontend build serve test-ui clean

install:
	python3 -m venv backend/.venv
	backend/.venv/bin/pip install -r backend/requirements.txt
	npm --prefix frontend install

setup-cert:
	bash backend/scripts/gen-cert.sh

dev-backend:
	cd backend && .venv/bin/flask --app "app:create_app()" run --port 8555 --with-threads

dev-frontend:
	npm --prefix frontend run dev

build:
	npm --prefix frontend run build

serve: build
	cd backend && .venv/bin/flask --app "app:create_app()" run --port 8555 --with-threads

test-ui:
	npm --prefix frontend run test:e2e

clean:
	rm -rf frontend/dist
	find backend -name __pycache__ -type d -exec rm -rf {} +
