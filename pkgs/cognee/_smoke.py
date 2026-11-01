"""Offline package smoke test, with all mutable storage in the build sandbox."""

import os
import sys
from pathlib import Path

import cognee
import ladybug
import lancedb
import onnxruntime
from cognee.base_config import BaseConfig
from cognee_db_workers._kuzu_helpers import load_json_extension
from fastapi.testclient import TestClient


assert cognee.__version__ == sys.argv[1], cognee.__version__
assert "CPUExecutionProvider" in onnxruntime.get_available_providers()

# Defaults must be writable user paths, while explicit configuration still wins.
for field, directory in (
    ("data_root_directory", "data"),
    ("system_root_directory", "system"),
    ("cache_root_directory", "cache"),
):
    assert BaseConfig.model_fields[field].default == str(Path.home() / ".cognee" / directory)
    assert getattr(BaseConfig(), field) == os.environ[field.upper()]

root = Path(os.environ["TMPDIR"])
vector_db = lancedb.connect(str(root / "vectors"))
table = vector_db.create_table("smoke", [{"vector": [1.0, 0.0], "text": "nix"}])
assert table.search([1.0, 0.0]).limit(1).to_list()[0]["text"] == "nix"

# Exercise the real native graph engine and Cognee's extension loader offline.
with ladybug.Database(str(root / "graph"), buffer_pool_size=64 * 1024 * 1024) as db:
    with ladybug.Connection(db) as connection:
        connection.execute(f"CALL home_directory = '{root}';")
        load_json_extension(connection.execute)
        result = connection.execute("RETURN json('{\"packaged\":true}')")
        assert result.get_next() == ['{"packaged":true}']

# A service needs a working API lifespan, including SQLite migrations.
from cognee.api.client import app

with TestClient(app) as client:
    response = client.get("/")
    assert response.status_code == 200, response.text
    assert response.json() == {"message": "Hello, World, I am alive!"}
