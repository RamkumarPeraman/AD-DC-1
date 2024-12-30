import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class OUController extends Controller {
  @tracked ous = [];
  @tracked selectedOU = null;
  @tracked searchQuery = '';
  @tracked sortBy = 'asc-desc';

  get filteredOUs() {
    let filteredOUs = this.ous;

    if (this.searchQuery) {
      filteredOUs = filteredOUs.filter(ou =>
        ou.ouName.toLowerCase().includes(this.searchQuery.toLowerCase())
      );
    }

    switch (this.sortBy) {
      case 'asc-desc':
        filteredOUs = filteredOUs.sort((a, b) => a.ouName.localeCompare(b.ouName));
        break;
      case 'desc-asc':
        filteredOUs = filteredOUs.sort((a, b) => b.ouName.localeCompare(a.ouName));
        break;
      case 'new-old':
        filteredOUs = filteredOUs.sort((a, b) => new Date(b.createdDate) - new Date(a.createdDate));
        break;
      case 'old-new':
        filteredOUs = filteredOUs.sort((a, b) => new Date(a.createdDate) - new Date(b.createdDate));
        break;
    }

    return filteredOUs;
  }

  get totalCount() {
    return this.filteredOUs.length;
  }

  @action
  updateSearchQuery(event) {
    this.searchQuery = event.target.value;
  }

  @action
  updateSortBy(event) {
    this.sortBy = event.target.value;
  }

  @action
  showOUDetails(ou) {
    this.selectedOU = ou;
  }

  @action
  closePopup() {
    this.selectedOU = null;
  }
}
