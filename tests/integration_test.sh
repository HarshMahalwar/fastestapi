#!/usr/bin/env bash
set -euo pipefail

BASE_URL="http://localhost:8080"
PASS=0
FAIL=0
BINARY="${1:-./build/example}"

cleanup() {
    if [ -n "${SERVER_PID:-}" ]; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f test.db
}
trap cleanup EXIT

assert_status() {
    local test_name="$1" expected="$2" actual="$3"
    if [ "$actual" -eq "$expected" ]; then
        echo "PASS: $test_name (HTTP $actual)"
        ((PASS++))
    else
        echo "FAIL: $test_name - expected HTTP $expected, got HTTP $actual"
        ((FAIL++))
    fi
}

assert_json_field() {
    local test_name="$1" body="$2" field="$3" expected="$4"
    actual=$(echo "$body" | python3 -c "import sys,json; print(json.load(sys.stdin).get('$field',''))" 2>/dev/null || echo "")
    if [ "$actual" = "$expected" ]; then
        echo "PASS: $test_name ($field=$actual)"
        ((PASS++))
    else
        echo "FAIL: $test_name - expected $field='$expected', got '$actual'"
        ((FAIL++))
    fi
}

assert_json_array_length_gte() {
    local test_name="$1" body="$2" min_len="$3"
    actual=$(echo "$body" | python3 -c "import sys,json; print(len(json.load(sys.stdin)))" 2>/dev/null || echo "0")
    if [ "$actual" -ge "$min_len" ]; then
        echo "PASS: $test_name (length=$actual >= $min_len)"
        ((PASS++))
    else
        echo "FAIL: $test_name - expected array length >= $min_len, got $actual"
        ((FAIL++))
    fi
}

# Start the server
echo "Starting server: $BINARY"
$BINARY &
SERVER_PID=$!
sleep 2

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Server failed to start"
    exit 1
fi
echo "  Server running (PID $SERVER_PID)"
echo ""

# Create an item
echo "Test: POST /items (Create)"
HTTP_CODE=$(curl -s -o /tmp/resp_create.json -w "%{http_code}" \
    -X POST "$BASE_URL/items" \
    -H "Content-Type: application/json" \
    -d '{"name":"Widget","price":9.99}')
BODY=$(cat /tmp/resp_create.json)
assert_status "Create item status" 201 "$HTTP_CODE" || \
    assert_status "Create item status (201 accepted)" 201 "$HTTP_CODE"
assert_json_field "Create item name" "$BODY" "name" "Widget"
echo ""

# Extract created item ID
ITEM_ID=$(echo "$BODY" | python3 -c "import sys,json; print(json.load(sys.stdin).get('id','1'))" 2>/dev/null || echo "1")

# List all items
echo "Test: GET /items (List)"
HTTP_CODE=$(curl -s -o /tmp/resp_list.json -w "%{http_code}" "$BASE_URL/items")
BODY=$(cat /tmp/resp_list.json)
assert_status "List items status" 200 "$HTTP_CODE"
assert_json_array_length_gte "List has items" "$BODY" 1
echo ""

# Get single item
echo "Test: GET /items/$ITEM_ID (Get one)"
HTTP_CODE=$(curl -s -o /tmp/resp_get.json -w "%{http_code}" "$BASE_URL/items/$ITEM_ID")
BODY=$(cat /tmp/resp_get.json)
assert_status "Get item status" 200 "$HTTP_CODE"
assert_json_field "Get item name" "$BODY" "name" "Widget"
echo ""

# Update item
echo "Test: PUT /items/$ITEM_ID (Update)"
HTTP_CODE=$(curl -s -o /tmp/resp_update.json -w "%{http_code}" \
    -X PUT "$BASE_URL/items/$ITEM_ID" \
    -H "Content-Type: application/json" \
    -d '{"price":12.50}')
BODY=$(cat /tmp/resp_update.json)
assert_status "Update item status" 200 "$HTTP_CODE"
echo ""

# Verify update
echo "Test: GET /items/$ITEM_ID (Verify update)"
HTTP_CODE=$(curl -s -o /tmp/resp_verify.json -w "%{http_code}" "$BASE_URL/items/$ITEM_ID")
BODY=$(cat /tmp/resp_verify.json)
assert_status "Verify update status" 200 "$HTTP_CODE"
assert_json_field "Verify updated price" "$BODY" "price" "12.5"
echo ""

# Delete item
echo "Test: DELETE /items/$ITEM_ID (Delete)"
HTTP_CODE=$(curl -s -o /tmp/resp_delete.json -w "%{http_code}" \
    -X DELETE "$BASE_URL/items/$ITEM_ID")
assert_status "Delete item status" 204 "$HTTP_CODE"
echo ""

# Verify deletion
echo "Test: GET /items/$ITEM_ID (Verify deletion)"
HTTP_CODE=$(curl -s -o /tmp/resp_gone.json -w "%{http_code}" "$BASE_URL/items/$ITEM_ID")
assert_status "Deleted item returns 404" 404 "$HTTP_CODE"
echo ""

# Non-existent item
echo "Test: GET /items/99999 (Not found)"
HTTP_CODE=$(curl -s -o /tmp/resp_notfound.json -w "%{http_code}" "$BASE_URL/items/99999")
assert_status "Non-existent item returns 404" 404 "$HTTP_CODE"
echo ""

# Invalid JSON
echo "Test: POST /items (Invalid JSON)"
HTTP_CODE=$(curl -s -o /tmp/resp_badjson.json -w "%{http_code}" \
    -X POST "$BASE_URL/items" \
    -H "Content-Type: application/json" \
    -d '{invalid json}')
assert_status "Invalid JSON returns 400" 400 "$HTTP_CODE"
echo ""

# Summary
echo "  Results: $PASS passed, $FAIL failed"

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi