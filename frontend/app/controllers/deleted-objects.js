import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class DeletedObjectsController extends Controller {
  @tracked deletedObjects = [];
  @tracked selectedDeletedObject = null;

  @action
  showDeletedObjectsDetails(deletedObject) {
    this.selectedDeletedObject = deletedObject;
  }

  @action
  closePopup() {
    this.selectedDeletedObject = null;
  }
}