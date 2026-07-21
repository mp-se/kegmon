<template>
  <button
    :id="id"
    type="button"
    class="btn btn-secondary"
    hidden
    data-bs-toggle="modal"
    :data-bs-target="'#modal' + $.uid"
  >
    Testing
  </button>
  <div class="modal fade modal-lg" :id="'modal' + $.uid" tabindex="-1" aria-hidden="true">
    <div class="modal-dialog">
      <div class="modal-content p-4">
        <div class="modal-header">
          <h1 class="modal-title fs-5">{{ title }}</h1>
        </div>
        <div class="modal-body">
          {{ message }}
        </div>
        <div class="modal-footer">
          <button
            @click="callback(true)"
            type="button"
            class="btn btn-primary"
            data-bs-dismiss="modal"
          >
            Confirm
          </button>
          <button
            @click="callback(false)"
            type="button"
            class="btn btn-secondary"
            data-bs-dismiss="modal"
          >
            Cancel
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
/**
 * Purpose: Show a yes/no dialog to confirm an action
 */
defineOptions({
  inheritAttrs: false
})

/**
 * Ref to callback where true/false will be a parameter (required)
 */
const callback = defineModel('callback')
/**
 * Ref to dialog message (required)
 */
const message = defineModel('message')
/**
 * Ref to dialog id (required)
 */
const id = defineModel('id')
/**
 * Modal title (required).
 */
const title = defineModel('title')
</script>
