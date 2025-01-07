import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class DeletedObjectsController extends Controller {
  @tracked deletedObjects = [];
  @tracked selectedDeletedObject = null;
  @tracked searchQuery = '';
  @tracked sortBy = 'asc-desc'; // Default sort order

  get filteredDeletedObjects() {
    if (!this.searchQuery) {
      return this.deletedObjects;
    }
    return this.deletedObjects.filter((obj) => {
      return (
        (obj.userName &&
          obj.userName
            .toLowerCase()
            .includes(this.searchQuery.toLowerCase())) ||
        (obj.groupName &&
          obj.groupName
            .toLowerCase()
            .includes(this.searchQuery.toLowerCase())) ||
        (obj.computerName &&
          obj.computerName
            .toLowerCase()
            .includes(this.searchQuery.toLowerCase())) ||
        (obj.ouName &&
          obj.ouName.toLowerCase().includes(this.searchQuery.toLowerCase()))
      );
    });
  }

  get sortedDeletedObjects() {
    let sortedObjects = [...this.filteredDeletedObjects];
    switch (this.sortBy) {
      case 'asc-desc':
        sortedObjects.sort((a, b) =>
          (
            a.userName ||
            a.groupName ||
            a.computerName ||
            a.ouName
          ).localeCompare(
            b.userName || b.groupName || b.computerName || b.ouName,
          ),
        );
        break;
      case 'desc-asc':
        sortedObjects.sort((a, b) =>
          (
            b.userName ||
            b.groupName ||
            b.computerName ||
            b.ouName
          ).localeCompare(
            a.userName || a.groupName || a.computerName || a.ouName,
          ),
        );
        break;
      case 'new-old':
        sortedObjects.sort((a, b) => new Date(b.date) - new Date(a.date));
        break;
      case 'old-new':
        sortedObjects.sort((a, b) => new Date(a.date) - new Date(b.date));
        break;
    }
    return sortedObjects;
  }

  get totalCount() {
    return this.filteredDeletedObjects.length;
  }

  @action
  showDeletedObjectsDetails(deletedObject) {
    this.selectedDeletedObject = deletedObject;
  }

  @action
  closePopup() {
    this.selectedDeletedObject = null;
  }

  @action
  updateSearchQuery(event) {
    this.searchQuery = event.target.value;
  }

  @action
  updateSortBy(event) {
    this.sortBy = event.target.value;
  }
}
